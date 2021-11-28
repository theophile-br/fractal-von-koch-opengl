cmake_minimum_required(VERSION 3.20)
project(fractal)

set(CMAKE_CXX_STANDARD 14)
include_directories(include)
add_executable(fractal main.cpp glad_gl.c)
target_link_libraries(fractal glfw)

file(COPY basic.shader DESTINATION ${CMAKE_BINARY_DIR})

