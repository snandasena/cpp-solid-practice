cmake_minimum_required(VERSION 3.10)
project(CppSOLIDInPractice)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -pedantic -Wall -Werror")


add_subdirectory(server)
add_subdirectory(tests/ut)