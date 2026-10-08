#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <arpa/inet.h>

#include "EventLoop.h"
#include "Config.h"
#include "decoder/Decoder.h"

std::atomic<bool> stopping{false};

void signal_handler(int)
{
    stopping = true;
}
int main(int argc, char **argv)
{

    signal(SIGPIPE, SIG_IGN);
    //监听服务器

    Config config;
    config.init(argc, argv);

    if(config.dump_config_)
    {
        config.print();
        return 0;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);

    try
    {
        unsigned int thread_count = config.thread_count;
        std::vector<std::unique_ptr<EventLoop>> loops;
        std::vector<std::thread> threads;

        for (unsigned int i = 0; i < thread_count; ++i)
        {
            auto loop = std::make_unique<EventLoop>(config);

            loop->setDecoder(std::make_unique<Decoder>());

            loop->set_message_callback(
                [](Connection &connection,
                   const std::string &message)
                {
                    uint32_t length = message.size();
                    length = htonl(length);

                    connection.output_buffer().add_data(
                        reinterpret_cast<const char *>(&length),
                        sizeof(length));

                    connection.output_buffer().add_data(
                        message.data(),
                        message.size());
                });

            loops.push_back(std::move(loop));
        }

        for (unsigned int i = 0; i < thread_count; ++i)
        {
            threads.emplace_back(
                [loop = loops[i].get(), i]()
                {
                    std::cout
                        << "[Thread " << i
                        << "] starting..." << std::endl;

                    loop->run();
                });
        }

        while (!stopping)
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(100));
        }

        for (auto &loop : loops)
        {
            loop->stop();
        }

        for (auto &thread : threads)
        {
            thread.join();
        }

        std::cout << "TCP server stopped" << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: "
                  << e.what() << std::endl;
        return 1;
    }
    
    
    return 0;
}