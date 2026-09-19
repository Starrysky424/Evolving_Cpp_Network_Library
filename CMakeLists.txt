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


list(REMOVE_ITEM LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_library(network_lib ${LIB_SOURCES})

add_executable(main ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(main network_lib pthread)


add_subdirectory(benchmark)
add_subdirectory(test)