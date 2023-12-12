find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "-ggdb3 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

add_executable(MMFAllocatorTest MMFAllocatorTest.cpp)
target_compile_options(MMFAllocatorTest PRIVATE -fsanitize=address -fsanitize=undefined -fsanitize=leak)
target_link_libraries(MMFAllocatorTest PRIVATE MMFAllocator::MMFAllocator gtest gtest_main asan ubsan)
add_test(NAME MMFAllocatorTest COMMAND MMFAllocatorTest)
