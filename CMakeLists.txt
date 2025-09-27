cmake_minimum_required(VERSION 3.20)

project(KrystalChip8 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(krystal main.cpp chip8.cpp)

file(COPY beep.wav DESTINATION ${CMAKE_BINARY_DIR})

target_compile_options(krystal PRIVATE -Wall -Wextra)

target_link_libraries(krystal PRIVATE raylib)