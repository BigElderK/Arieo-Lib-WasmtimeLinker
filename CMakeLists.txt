cmake_minimum_required (VERSION 3.31)
include($ENV{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)

ARIEO_PACKAGE(
    Arieo-Lib-WasmtimeLinker
    CATEGORY 02_engine
)

ARIEO_ENGINE_PROJECT(
    arieo_wasmtime_linker_lib

    PROJECT_TYPE headonly_library

    DEPENDENCIES
        ARIEO_PACKAGES
            Arieo-Core
        INTERFACES
            Arieo-Core::arieo_core
    SOURCES
        INTERFACE_INCLUDE_FOLDERS
            ${CMAKE_CURRENT_SOURCE_DIR}/public/include
)
