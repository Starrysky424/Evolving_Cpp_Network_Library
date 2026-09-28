#include"EventLoop.h"
#include<iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include<fcntl.h>
#include"logger.h"
#include<chrono>
#include"Connection.h"
#include<sys/eventfd.h>
EventLoop::EventLoop(int server_fd)
    : server_fd_(server_fd),
      wakeup_fd_(eventfd(0,EFD_NONBLOCK)),
      timer_queue_(this),
      connectionManager_(&epollPoller_),
      running_(true)
{
    if(wakeup_fd_==-1)
    {
        throw std::runtime_error("eventfd failed");
    }
    epollPoller_.add_fd(server_fd_);
    epollPoller_.add_fd(wakeup_fd_);
    epollPoller_.add_fd(timer_queue_.getTimerFd());

    startIdleTimeout();
    LOG_INFO("TCP server initialized");
}

    // 有新客户端连接
    void EventLoop::accept_new_connection()
    {
        
        while(true)
        {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);

            int client_fd = accept(
                server_fd_, (sockaddr *)&client_addr, &client_len);

            if (client_fd == -1)
            {
                if(errno==EAGAIN || errno==EWOULDBLOCK)
                    break;
                perror("accept");
                break;
            }
            // 设置非阻塞
            int flags = fcntl(client_fd, F_GETFL, 0);

            if (flags == -1)
            {
                close(client_fd);
                return;
            }

            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

            connectionManager_.add_connection(client_fd);

            Connection *client = connectionManager_.get_connection(client_fd);

            client->setMessageCallback(message_callback_);

            client->setCloseCallback(
                [this](int fd)
                {
                    connectionManager_.delete_connection(fd);
                });
            client->setWriteCallbacks(
                [this, client_fd, client]()
                {
                    if(client->enable_write())
                    {
                        epollPoller_.modify_fd(client_fd, client->events());

                    }
                },
                [this, client_fd, client]()
                {
                    if(client->disable_write())
                    {
                        epollPoller_.modify_fd(client_fd, client->events());
                    }
                });
            std::cout << "new client:" << client_fd << std::endl;
        }
       
       
    }

    //处理客户端

    void EventLoop::handle_client_event(int fd)
    {
        Connection *client = connectionManager_.get_connection(fd);

        if (client == nullptr)
            return;

        IOEvent event = client->recv_data();

        if (event==IOEvent::CLOSE || event==IOEvent::ERROR)
        {
            return;
        }

        if(event==IOEvent::DATA)
        {
            client->send_data();
        }
    }

void EventLoop::run()
{
    loop_thread_id_ = std::this_thread::get_id();

    while (running_)
    {
        int n = epollPoller_.wait(5000);

      
        if (n == -1)
        {
            perror("epoll_wait");
            continue;
        }
        else if (n == 0)
        {
            std::cout << "epoll timeout" << std::endl;
            continue;
        }

        auto events = get_events(n);

        for(auto &event:events)
        {
            
            if(event.type==EventType::NEW_CONNECTION)
            {
                accept_new_connection();
            }

            else if(event.type==EventType::READ)
            {
               
                handle_client_event(event.fd);
            }

            else if(event.type==EventType::TIMER)
            {
                timer_queue_.handleRead();
            }

            else if(event.type==EventType::WRITE)
            {
                Connection *client = connectionManager_.get_connection(event.fd);
                
                if(client==nullptr)
                    continue;

                client->send_data();

            }

            else if(event.type==EventType::WAKEUP)
            {
                uint64_t value;
                read(wakeup_fd_, &value, sizeof(value));

                std::vector<std::function<void()>> local_tasks;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    local_tasks.swap(tasks_);
                }

                for(auto &task:local_tasks)
                {
                    task();
                }
            }
        }
    }
}



std::vector<Event> EventLoop:: get_events(int n)
{
    std::vector<Event> events;
    const auto &ready_events = epollPoller_.get_ready_events();

    for (int i = 0; i < n;i++)
    {
        int fd = ready_events[i].data.fd;
        uint32_t revents = ready_events[i].events;

        if (fd == server_fd_)
        {
            events.emplace_back(fd, EventType::NEW_CONNECTION);
        }

        else if(fd==timer_queue_.getTimerFd())
        {
            events.emplace_back(fd, EventType::TIMER);
        }

        else if(fd==wakeup_fd_)
        {
            events.emplace_back(fd, EventType::WAKEUP);
        }
        else
        {
            if(revents&EPOLLIN)
            {
                events.emplace_back(fd, EventType::READ);
            }

            if (revents & EPOLLOUT)
            {
                events.emplace_back(fd, EventType::WRITE);
            }
        }
    }
    return events;
}

void EventLoop:: set_message_callback(ClientMessageCallback callback)
{
    message_callback_ = callback;
}

void EventLoop:: runAfter(std::chrono::milliseconds delay, TimerQueue::TimerCallback cb)
{
    timer_queue_.addTimer(std::move(cb), std::chrono::steady_clock::now() + delay, std::chrono::milliseconds(0));
}

void EventLoop:: runEvery(std::chrono::milliseconds interval, TimerQueue::TimerCallback cb)
{
    timer_queue_.addTimer(std::move(cb), std::chrono::steady_clock::now() + interval, interval);
}

void EventLoop::startIdleTimeout()
{
    runEvery(
        std::chrono::seconds(5),
        [this]()
        {
            auto now = std::chrono::steady_clock::now();
            auto timeout = std::chrono::seconds(60);

            std::vector<int> expired_fds;

            connectionManager_.forEachConn(
                [&](Connection *conn)
                {
                    if(now-conn->getLastActiveTime()>timeout)
                    {
                        LOG_INFO(
                            "[IdleTimeout] fd:%d idle timeout, close",
                            conn->fd());

                        expired_fds.push_back(conn->fd());
                    }
                });
            
            for(int fd:expired_fds)
            {
                connectionManager_.delete_connection(fd);

            }
        });
}

void  EventLoop:: queueInLoop(std::function<void()> cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push_back(std::move(cb));
    }

    uint64_t cnt = 1;
    write(wakeup_fd_, &cnt, sizeof(cnt));
}

void EventLoop::runInLoop(std::function<void()> cb)
{
    if (std::this_thread::get_id() == loop_thread_id_)
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::stop()
{
    running_ = false;

    uint64_t value = 1;
    write(wakeup_fd_, &value, sizeof(value));
}