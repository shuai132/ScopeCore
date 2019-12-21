cmake_minimum_required(VERSION 3.1)

project(ScopeCore)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall)

add_library(${PROJECT_NAME} STATIC Scope.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC .)
