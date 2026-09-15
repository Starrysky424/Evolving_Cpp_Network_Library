cmake_minimum_required(VERSION 3.10)

project(EvolvingCNetworkLibrary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g") 

include_directories(include)

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(main ${SOURCES})

add_subdirectory(benchmark)
