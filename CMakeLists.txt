cmake_minimum_required (VERSION 3.31)

arieo_engine_project(
    arieo_wasmtime_linker_lib
    ALIAS arieo::lib::wasmtime_linker
    PROJECT_TYPE headonly_library
    PUBLIC_INCLUDE_FOLDERS
        ${CMAKE_CURRENT_SOURCE_DIR}/public/include
)