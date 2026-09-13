cmake_minimum_required(VERSION 3.10)

project(EvolvingCNetworkLibrary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(main ${SOURCES})