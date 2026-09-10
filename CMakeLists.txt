cmake_minimum_required(VERSION 3.10)

project(EvolvingCNetworkLibrary)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

set(
    SOURCES
     src/main.cpp
    src/Buffer.cpp
    src/Connection.cpp
    src/ConnectionManager.cpp
)

add_executable(main ${SOURCES})