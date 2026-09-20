cmake_minimum_required(VERSION 3.10)

add_executable(test_buffer
    test_buffer.cpp
)

target_link_libraries(test_buffer
    network_lib
    pthread
)

add_executable(test_decoder
    test_decoder.cpp
)

target_link_libraries(test_decoder
    network_lib
    pthread
)

add_executable(test_connection_manager
    test_connection_manager.cpp
)

target_link_libraries(test_connection_manager
    network_lib
    pthread
)