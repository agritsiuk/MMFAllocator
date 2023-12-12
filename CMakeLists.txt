cmake_minimum_required(VERSION 3.10)
project(mmf_allocator)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_library(MMFAllocator INTERFACE)
add_library(MMFAllocator::MMFAllocator ALIAS MMFAllocator)
target_include_directories(MMFAllocator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory(tests)
