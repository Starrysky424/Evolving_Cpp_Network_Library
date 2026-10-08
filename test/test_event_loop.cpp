#include "EventLoop.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    
    Config config;
    config.max_events = 4096;
    config.epoll_timeout_ms = 100;
    config.recv_buffer_size = 4096;
    EventLoop loop(config);

    std::atomic<bool> loop_thread_task_executed{false};
    std::atomic<bool> other_thread_task_executed{false};

    // 由其他线程先提交一个任务
    // 让 EventLoop 线程启动后执行
    loop.queueInLoop([&]()
                     {
        std::cout << "EventLoop thread task started." << std::endl;

        // 此时已经处于 EventLoop 线程
        // runInLoop() 应该直接执行
        loop.runInLoop([&]()
        {
            loop_thread_task_executed = true;
            std::cout << "runInLoop direct task executed." << std::endl;
        }); });

    // EventLoop线程
    std::thread loop_thread([&]()
                            { loop.run(); });

    // 等待 EventLoop 启动并处理第一个任务
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 当前是主线程
    // runInLoop() 应该进入 queueInLoop()
    // 然后通过 eventfd 唤醒 EventLoop
    loop.runInLoop([&]()
                   {
        other_thread_task_executed = true;
        std::cout << "runInLoop cross-thread task executed." << std::endl; });

    // 等待任务执行
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(loop_thread_task_executed == true);
    assert(other_thread_task_executed == true);

    std::cout << "runInLoop test passed." << std::endl;

    // 停止 EventLoop
    loop.stop();

    // 等待 EventLoop线程退出
    loop_thread.join();

   

    std::cout << "EventLoop test finished." << std::endl;

    return 0;
}