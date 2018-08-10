cmake_minimum_required(VERSION 3.5.1)

project(UDPServer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 17)

add_executable(udpclientest udpclientest.cpp)
add_executable(udptest test.cpp UDPServer)
add_library(UDPServer STATIC udpserver.cpp udpserver.h udpserver.h)


