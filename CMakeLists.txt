cmake_minimum_required(VERSION 3.5.1)

project(UDPServer VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_FLAGS "-g -Werror=unused-parameter")
set(CMAKE_CXX_STANDARD 17)

add_executable(udpclientest udpclientest.cpp udpclient.cpp)
add_executable(udpservertest udpservertest.cpp udpserver.cpp)

add_library(UDPServer STATIC udpserver.cpp udpserver.h udpclient.cpp udpclient.h)

