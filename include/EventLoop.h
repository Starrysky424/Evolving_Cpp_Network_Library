#pragma once

#include"EpollPoller.h"
#include<functional>
#include"ConnectionManager.h"
#include"Event.h"
#include<string>
#include"TimerQueue.h"
#include<mutex>
#include<vector>
#include<thread>
#include "decoder/FrameDecoder.h"
#include"Config.h"
#include "SocketListener.h"
class EventLoop
{
public:
    using ClientMessageCallback = std::function<void(Connection &, const std::string &)>;
    EventLoop(Config& config);
    ~EventLoop();
    void run();

    void set_message_callback(ClientMessageCallback callback);

    TimerQueue::TimerId runAt(std::chrono::steady_clock::time_point when, TimerQueue::TimerCallback cb);

    TimerQueue::TimerId runAfter(std::chrono::milliseconds delay, TimerQueue::TimerCallback cb);

    TimerQueue::TimerId runEvery(std::chrono::milliseconds interval, TimerQueue::TimerCallback cb);

    void queueInLoop(std::function<void()> cb);

    void runInLoop(std::function<void()> cb);

    void stop();

    void setDecoder(std::unique_ptr<FrameDecoder> decoder);

    void cancelTimer(TimerQueue::TimerId timerId);

private:
    void accept_new_connection();
    void handle_client_event(int fd);

    std::vector<Event> get_events(int n);

    void startIdleTimeout();

private:
    SocketListener socketListener_;
    int wakeup_fd_;
    EpollPoller epollPoller_;
    ConnectionManager connectionManager_;

    ClientMessageCallback message_callback_;
    TimerQueue timer_queue_;
    std::thread::id loop_thread_id_;
    std::vector<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::unique_ptr<FrameDecoder> decoder_;
    bool running_;
    int epoll_timeout_ms_;
};