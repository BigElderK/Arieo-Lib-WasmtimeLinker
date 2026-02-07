cmake_minimum_required (VERSION 3.31)

include($ENV{ARIEO_PACKAGE_BUILDENV_INSTALL_FOLDER}/cmake/build_environment.cmake)
project(ArieoEngine)
arieo_engine_project(
    arieo_wasmtime_linker_lib
    ALIAS arieo::lib::wasmtime_linker
    PROJECT_TYPE headonly_library
    PUBLIC_INCLUDE_FOLDERS
        ${CMAKE_CURRENT_LIST_DIR}/public/include
)