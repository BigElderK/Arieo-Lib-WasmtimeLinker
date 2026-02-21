cmake_minimum_required (VERSION 3.31)
include($ENV{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)

ARIEO_PACKAGE(
    Arieo-Lib-WasmtimeLinker
    CATEGORY 02_engine
    COMPONENTS
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
