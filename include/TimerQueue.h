#pragma once

#include <chrono>
#include <functional>
#include <queue>
#include <vector>

class EventLoop;

class TimerQueue
{
public:
    using TimerCallback = std::function<void()>;

    struct TimerId
    {
        int64_t sequence;
    };

    TimerQueue(EventLoop *loop);

    ~TimerQueue();


    //添加定时器
    TimerId addTimer(
        TimerCallback cb,
        std::chrono::steady_clock::time_point when,
        std::chrono::milliseconds interval);

        //取消定时器
    void cancel(TimerId timerId);

    // 给epoll监听
    int getTimerFd() const;

    // timerfd触发后调用
    void handleRead();

private:

    struct Timer
    {
        TimerId id;

        std::chrono::steady_clock::time_point when;

        TimerCallback cb;

        std::chrono::milliseconds interval;

        bool operator>(const Timer &rhs) const
        {
            return when > rhs.when;
        }
    };

    using TimerHeap =
        std::priority_queue<
            Timer,
            std::vector<Timer>,
            std::greater<Timer>>;

    int createTimerfd();

    //修改时间
    void resetTimerfd(
        std::chrono::steady_clock::time_point when);

private:
    EventLoop *loop_;

    int timerfd_;

    TimerHeap heap_;

    int64_t seq_counter_;
};