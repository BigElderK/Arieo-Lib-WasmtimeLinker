cmake_minimum_required (VERSION 3.31)

include($env{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)
project(ArieoEngine)
arieo_engine_project(
    arieo_wasmtime_linker_lib
    PROJECT_TYPE headonly_library

    SOURCES
        INTERFACE_INCLUDE_FOLDERS
            ${CMAKE_CURRENT_LIST_DIR}/public/include
)
