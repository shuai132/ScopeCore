cmake_minimum_required(VERSION 3.1)

project(ScopeCore)

# PacketProcessor
add_subdirectory(modules/PacketProcessor)
link_libraries(packetprocessor)
target_compile_definitions(packetprocessor PRIVATE NDEBUG)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall)

add_library(ScopeCoreMCU STATIC ScopeMCU.cpp)
target_include_directories(ScopeCoreMCU PUBLIC .)

add_library(ScopeCoreGUI STATIC ScopeGUI.cpp)
target_include_directories(ScopeCoreGUI PUBLIC .)
