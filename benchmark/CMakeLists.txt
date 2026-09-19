add_executable(tcp_benchmark tcp_benchmark.cpp)


target_link_libraries(tcp_benchmark
    network_lib
    pthread
)
