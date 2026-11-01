cmake_minimum_required(VERSION 3.16)

project(cns
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(cns
    src/main.cpp
    src/lexer.cpp
)

target_include_directories(cns PRIVATE include)

target_compile_options(cns PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
