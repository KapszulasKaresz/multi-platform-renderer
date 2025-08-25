cmake_minimum_required(VERSION 3.18)
project(MultiPlatformRenderer C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(app)
add_subdirectory(libs)