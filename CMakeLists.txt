cmake_minimum_required(VERSION 3.18)
project(MultiPlatformRenderer C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${PROJECT_NAME})
add_library(renderer::renderer ALIAS ${PROJECT_NAME})

include(cmake/dependencies.cmake)

target_include_directories(${PROJECT_NAME} PUBLIC "libs")

add_subdirectory(app)
add_subdirectory(libs)