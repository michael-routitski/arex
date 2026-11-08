cmake_minimum_required(VERSION 3.20)

project(
    arex
    VERSION 0.1.0
    DESCRIPTION "Ultra-fast line-oriented terminal directory utility"
    LANGUAGES CXX
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are forbidden. Create a build/ directory or use CMakePresets.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerFlags)
include(Sanitizers)

# Core Library Target
add_library(arex_core STATIC
    src/terminal_canvas.cpp
    src/file_previewer.cpp
    src/process_runner.cpp
    src/key_listener.cpp
    src/directory_scanner.cpp
)

add_library(arex::core ALIAS arex_core)

target_include_directories(arex_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_apply_warnings(arex_core)
target_enable_sanitizers(arex_core)

# Executable Target
add_executable(arex_app apps/main.cpp)
add_executable(arex::app ALIAS arex_app)
set_target_properties(arex_app PROPERTIES OUTPUT_NAME "arex")

target_link_libraries(arex_app PRIVATE arex::core)
target_apply_warnings(arex_app)
target_enable_sanitizers(arex_app)

# Testing
option(BUILD_TESTING "Build unit test suite" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
