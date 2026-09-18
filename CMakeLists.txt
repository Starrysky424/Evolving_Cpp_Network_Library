cmake_minimum_required(VERSION 3.10)

project(EvolvingCNetworkLibrary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g") 

include_directories(${PROJECT_SOURCE_DIR}/include)

#网络库源码，不包含main.cpp
file(GLOB LIB_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_library(network_lib ${LIB_SOURCES})

add_subdirectory(benchmark)
