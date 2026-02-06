#pragma once

#include "core/module/module.h"

#include <wasmtime.hh>
#include <wasmtime/component.hh>
#include <string>
#include <cctype>


namespace Arieo::Lib::WasmtimeLinker 
{
    // Extract parameter types from function signature
    template<typename Ret, typename Class, typename... Args>
    auto extractParams(Ret(Class::*)(Args...)) -> std::tuple<Args...>;

    // Helper to get index sequence for tuple
    template<typename Tuple>
    struct tuple_index_sequence;

    template<typename... Args>
    struct tuple_index_sequence<std::tuple<Args...>> {
        using type = std::index_sequence_for<Args...>;
    };

    // Helper to extract value from WASM Val based on type
    template<typename T>
    T extractValue(const wasmtime::component::Val& val, std::size_t index)
    {
        if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, int>) {
            if (val.is_s32()) {
                return val.get_s32();
            }
        }
        else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, long long>) {
            if (val.is_s64()) {
                return val.get_s64();
            }
            // Also try u64 for int64_t (handle/pointer types from WASM)
            if (val.is_u64()) {
                return static_cast<int64_t>(val.get_u64());
            }
        }
        else if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, unsigned long long>) {
            if (val.is_u64()) {
                return val.get_u64();
            }
        }
        else if constexpr (std::is_same_v<T, float>) {
            if (val.is_f32()) {
                return val.get_f32();
            }
        }
        else if constexpr (std::is_same_v<T, double>) {
            if (val.is_f64()) {
                return val.get_f64();
            }
        }
        return T{};
    }

    // Helper to convert C++ return value to WASM Val type
    template<typename Ret>
    wasmtime::component::Val createResultVal(const Ret& result)
    {
        if constexpr (std::is_same_v<Ret, int32_t> || std::is_same_v<Ret, int>) {
            return wasmtime::component::Val(static_cast<int32_t>(result));
        }
        else if constexpr (std::is_same_v<Ret, int64_t> || std::is_same_v<Ret, long long>) {
            return wasmtime::component::Val(static_cast<int64_t>(result));
        }
        else if constexpr (std::is_same_v<Ret, uint64_t> || std::is_same_v<Ret, unsigned long long>) {
            return wasmtime::component::Val(static_cast<uint64_t>(result));
        }
        else if constexpr (std::is_same_v<Ret, float>) {
            return wasmtime::component::Val(result);
        }
        else if constexpr (std::is_same_v<Ret, double>) {
            return wasmtime::component::Val(result);
        }
        return wasmtime::component::Val(static_cast<int32_t>(0));
    }

    // Define the interface create function callback
    using InterfaceCreateFunctionHostCallback = std::function<std::uint64_t(
        std::uint64_t, std::uint64_t, std::string_view
    )>;

    // Define the function signature type
    using InterfaceFunctionHostCallback = std::function<wasmtime::Result<std::monostate>(
        wasmtime::Store::Context, 
        const wasmtime::component::FuncType&,       
        wasmtime::Span<wasmtime::component::Val>,
        wasmtime::Span<wasmtime::component::Val>
    )>;

    // Generate callback using parameter pack expansion
    template<typename FuncPtr, typename Ret, typename Class, typename... Args, std::size_t... Is>
    InterfaceFunctionHostCallback generateCallbackImpl(FuncPtr func_ptr, Ret(Class::*)(Args...), std::index_sequence<Is...>)
    {
        return [func_ptr](
            wasmtime::Store::Context store_ctx, 
            const wasmtime::component::FuncType& func_type,
            wasmtime::Span<wasmtime::component::Val> args,
            wasmtime::Span<wasmtime::component::Val> results) -> wasmtime::Result<std::monostate> {
            
            Core::Logger::info("Generated callback invoked with {} args", args.size());
            
            // Extract instance pointer from first parameter (args[0]) as int64
            if (args.size() < 1 + sizeof...(Args)) {
                Core::Logger::error("Insufficient arguments: expected {}, got {}", 1 + sizeof...(Args), args.size());
                return wasmtime::Result<std::monostate>(std::monostate{});
            }
            
            int64_t instance_ptr_value = extractValue<int64_t>(args[0], 0);
            Class* instance = reinterpret_cast<Class*>(instance_ptr_value);
            
            if (!instance) {
                Core::Logger::error("Invalid instance pointer: {}", instance_ptr_value);
                return wasmtime::Result<std::monostate>(std::monostate{});
            }
            
            Core::Logger::trace("Instance pointer: 0x{:x}", instance_ptr_value);
            
            // Log extracted parameters directly
            ((Core::Logger::trace("Param {}: type={}, value={}", Is, typeid(Args).name(), extractValue<Args>(args[Is + 1], Is + 1))), ...);
            
            // Call the member function directly with parameter pack expansion
            if constexpr (std::is_void_v<Ret>) {
                (instance->*func_ptr)(extractValue<Args>(args[Is + 1], Is + 1)...);
            } else {
                Ret result = (instance->*func_ptr)(extractValue<Args>(args[Is + 1], Is + 1)...);
                Core::Logger::trace("Function returned: {}", result);
                
                // Store result in results span
                if (results.size() > 0) {
                    results[0] = createResultVal(result);
                }
            }
            
            return wasmtime::Result<std::monostate>(std::monostate{});
        };
    }

    // Main entry point to generate callback
    template<typename FuncPtr>
    InterfaceFunctionHostCallback generateCallback(FuncPtr func_ptr)
    {
        using FuncType = std::remove_pointer_t<FuncPtr>;
        using IndexSeq = typename tuple_index_sequence<decltype(extractParams(FuncType{}))>::type;
        return generateCallbackImpl(func_ptr, FuncType{}, IndexSeq{});
    }

    
}

namespace Arieo::Lib::WasmtimeLinker 
{
    struct InterfaceFunctionExportInfo
    {
        const char* m_function_name;
        uint64_t m_function_id;
        uint64_t m_function_checksum;
        InterfaceFunctionHostCallback m_host_callback;
    };

    struct InterfaceExportInfo
    {
        const char* m_interface_name;
        uint64_t m_interaface_id;
        uint64_t m_interface_checksum;
        std::size_t m_interface_type_hash;
        InterfaceFunctionExportInfo* m_member_function_array;
        size_t m_member_function_count;
    };

    struct LinkerExportInfo
    {
        InterfaceExportInfo* m_interface_array;
        size_t m_interface_count;
    };

    template<class T>
    class InterfaceExportInfoRegister
    {
    public:
        static void fillInterfaceExportInfo(InterfaceExportInfo& interface_export_info)
        {
            static std::string interface_name = Arieo::Base::InterfaceInfo<T>::getWitFullInterfaceName();

            static std::array<InterfaceFunctionExportInfo, Arieo::Base::InterfaceInfo<T>::getMemberFunctionCount()> function_info_array;
            static std::array<std::string, Arieo::Base::InterfaceInfo<T>::getMemberFunctionCount()> function_name_array;

            size_t function_index = 0;
            Arieo::Base::InterfaceInfo<T>::iteratorMemberFunctions(
                [&function_index](auto func_ptr, std::string_view func_name, std::string_view wit_func_name, std::uint64_t function_id, std::uint64_t function_checksum) 
                {
                    function_name_array[function_index] = std::string(wit_func_name);
                    function_info_array[function_index] = 
                    {
                        function_name_array[function_index].data(),
                        function_id,
                        function_checksum,
                        generateCallback(func_ptr) // Placeholder for callback
                    };
                    ++function_index;
                }
            );

            interface_export_info.m_interface_name = interface_name.data();
            interface_export_info.m_interaface_id = Arieo::Base::InterfaceInfo<T>::getInterfaceId();
            interface_export_info.m_interface_checksum = Arieo::Base::InterfaceInfo<T>::getInterfaceChecksum();
            interface_export_info.m_interface_type_hash = Arieo::Base::ct::genCrc32StringID(typeid(T).name());
            
            interface_export_info.m_member_function_array = function_info_array.data();
            interface_export_info.m_member_function_count = function_info_array.size();
        }
    };

    template<class... Interfaces>
    class LinkerExportInfoRegister
    {
    public:
        static LinkerExportInfo* generateLinkerExportInfo()
        {
            static std::array<InterfaceExportInfo, sizeof...(Interfaces)> m_interface_export_info_array;
            static LinkerExportInfo m_linker_export_info;

            size_t index = 0;
            // Use initializer list to expand parameter pack
            (void)std::initializer_list<int>{
                (InterfaceExportInfoRegister<Interfaces>::fillInterfaceExportInfo(m_interface_export_info_array[index++]), 0)...
            };
            
            m_linker_export_info.m_interface_array = m_interface_export_info_array.data();
            m_linker_export_info.m_interface_count = sizeof...(Interfaces);
            return &m_linker_export_info;
        }
    };

    typedef LinkerExportInfo* (*DLLExportLinkInterfacesFn)(std::uint64_t version_checksum);
}
