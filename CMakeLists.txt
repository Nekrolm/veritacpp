cmake_minimum_required(VERSION 3.20)

project(VeritaCPP)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

include(CTest)

add_subdirectory(src)
add_subdirectory(tests)
