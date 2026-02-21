cmake_minimum_required (VERSION 3.31)

set(ARIEO_PACKAGE_CATEGORY "02_engine" CACHE INTERNAL "Category for Arieo packages")
set(ARIEO_PACKAGE_NAME "Arieo-Lib-WasmtimeLinker" CACHE INTERNAL "Name for Arieo package")
add_custom_target(
    Arieo-Lib-WasmtimeLinker
    DEPENDS 
        arieo_wasmtime_linker_lib
)


include($ENV{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)
project(ArieoEngine)
arieo_engine_project(
    arieo_wasmtime_linker_lib
    PROJECT_TYPE headonly_library

    SOURCES
        INTERFACE_INCLUDE_FOLDERS
            ${CMAKE_CURRENT_SOURCE_DIR}/public/include
)
