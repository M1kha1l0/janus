# project settings
cmake_minimum_required(VERSION 3.28)

project(janus
    VERSION 0.1.0
    DESCRIPTION "High-performance transparent proxy router"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(janus_warnings INTERFACE)
target_compile_options(janus_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -Wshadow
)
# Фикс несовместимости Clang < 19 и libstdc++ для <expected>
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
    target_compile_definitions(janus_warnings INTERFACE __cpp_concepts=202002L)
endif()

# sanitaizers
option(JANUS_ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)

if(JANUS_ENABLE_SANITIZERS)
    target_compile_options(janus_warnings INTERFACE 
        -fsanitize=address,undefined 
        -fno-omit-frame-pointer
    )
    target_link_options(janus_warnings INTERFACE 
        -fsanitize=address,undefined
    )
endif()

# libraries 
add_library(janus_core STATIC
    src/core.cpp
)

target_include_directories(janus_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(janus_core PUBLIC janus_warnings)

# binary file
add_executable(janus src/main.cpp)
target_link_libraries(janus PRIVATE janus_core)

# tests
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
    googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()