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
    class EventLoop
    {
    public:
        using ClientMessageCallback = std::function<void(Connection &, const std::string &)>;
        EventLoop(int server_fd);

        void run();

       
        void set_message_callback(ClientMessageCallback callback);

        void runAfter(std::chrono::milliseconds delay, TimerQueue::TimerCallback cb);

        void runEvery(std::chrono::milliseconds interval, TimerQueue::TimerCallback cb);

        void queueInLoop(std::function<void()> cb);

        void runInLoop(std::function<void()> cb);

        void stop();

    private:
        void accept_new_connection();
        void handle_client_event(int fd);

        std::vector<Event> get_events(int n);

        void startIdleTimeout();

    private:
        int server_fd_;
        int wakeup_fd_;
        EpollPoller epollPoller_;
        ConnectionManager connectionManager_;

        ClientMessageCallback message_callback_;
        TimerQueue timer_queue_;
        std::thread::id loop_thread_id_;
        std::vector<std::function<void()>> tasks_;
        std::mutex mutex_;

        bool running_;
    };