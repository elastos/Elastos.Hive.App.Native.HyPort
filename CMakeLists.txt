cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5)
project(Elastos.NET.Hive.App.Native.HyPort
    VERSION 1.0.0
    LANGUAGES C)

if(CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "Cross compiling is not supported.")
endif()

string(TOUPPER ${CMAKE_SYSTEM_NAME} CANONICAL_SYSTEM_NAME)
set(${CANONICAL_SYSTEM_NAME} TRUE)
if (NOT LINUX)
    message(FATAL_ERROR "Only Linux is supported.")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(HiveDefaults)
include(AddSubModule)

add_subdirectory(deps)
add_subdirectory(src)
