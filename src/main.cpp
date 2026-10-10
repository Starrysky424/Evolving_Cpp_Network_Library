#include "Config.h"
#include "EventLoop.h"
#include "decoder/DelimiterDecoder.h"

#include <signal.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

std::atomic<bool> stopping{false};

void signal_handler(int) {
    stopping = true;
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    Config config;
    config.init(argc, argv);

    if (config.dump_config_) {
        config.print();
        return 0;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        unsigned int thread_count = config.thread_count;

        std::vector<std::unique_ptr<EventLoop>> loops;
        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < thread_count; ++i) {
            auto loop = std::make_unique<EventLoop>(config);

            loop->setDecoder(std::make_unique<DelimiterDecoder>("\r\n\r\n"));

            loop->set_message_callback([](Connection& connection, const std::string& message) {
                std::string response;

                if (message.find("HTTP/") != std::string::npos) {
                    response =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 12\r\n"
                        "Connection: keep-alive\r\n"
                        "\r\n"
                        "hello,world\n";
                } else {
                    // 普通 TCP 消息：原样回显
                    response = message;
                }

                connection.output_buffer().add_data(response.data(), response.size());
            });

            loops.push_back(std::move(loop));
        }

        for (unsigned int i = 0; i < thread_count; ++i) {
            threads.emplace_back([loop = loops[i].get(), i]() {
                // std::cout
                //     << "[Thread " << i
                //     << "] starting..." << std::endl;

                loop->run();
            });
        }

        while (!stopping) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        for (auto &loop : loops) {
            loop->stop();
        }

        for (auto &thread : threads) {
            thread.join();
        }

        // std::cout << "TCP server stopped" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;

        return 1;
    }

    return 0;
}