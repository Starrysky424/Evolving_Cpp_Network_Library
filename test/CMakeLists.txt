cmake_minimum_required(VERSION 3.10)

add_executable(test_buffer
    test_buffer.cpp
)

target_link_libraries(test_buffer
    network_lib
    pthread
)