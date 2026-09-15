#include"EventLoop.h"
#include<iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include<fcntl.h>
EventLoop::EventLoop(int server_fd)
    : server_fd_(server_fd)
    {
        selectPoller_.add_fd(server_fd_);
    }

    // 有新客户端连接
    void EventLoop::accept_new_connection()
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(
            server_fd_, (sockaddr *)&client_addr, &client_len);

        if (client_fd == -1)
        {
            perror("accept");
            return;
        }

        //设置非阻塞
        int flags = fcntl(client_fd, F_GETFL, 0);

        if(flags==-1)
        {
            perror("fcntl");
            close(client_fd);
            return;
        }

        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        selectPoller_.add_fd(client_fd);
        connectionManager_.add_connection(client_fd);

        std::cout << "new client:" << client_fd << std::endl;
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
            std::cout << "server close fd: " << fd
                      << ", event: " << static_cast<int>(event)
                      << std::endl;
            connectionManager_.delete_connection(fd);
            selectPoller_.remove_fd(fd);
            close(fd);
            
        }

        else{
            if(message_callback_)
                message_callback_(*client,
                client->input_buffer());
        }
    }

void EventLoop::run()
{
    long long total_events = 0;
    long long total_reads = 0;
    while (true)
    {
        int n = selectPoller_.poll(5);

        int cnt = 0;
        cnt++;
        if (cnt % 5 == 0)
            std::cout << "ready events: " << n << std::endl;
        if (n == -1)
        {
            perror("select");
            continue;
        }
        else if (n == 0)
        {
            std::cout << "select timeout" << std::endl;
            continue;
        }

        auto events = get_events();

        for(auto &event:events)
        {
            total_events++;

         

            if(event.type==EventType::NEW_CONNECTION)
            {
                accept_new_connection();
            }

            else if(event.type==EventType::READ)
            {
                total_reads++;
                handle_client_event(event.fd);
            }
        }
    }
}



std::vector<Event> EventLoop:: get_events()
{
    std::vector<Event> events;

    for(int fd:selectPoller_.get_ready_fds())
    {
        if(fd==server_fd_)
        {
            events.emplace_back(fd, EventType::NEW_CONNECTION);
        }

        else
        {
            events.emplace_back(fd, EventType::READ);
        }
    }
    return events;
}

void EventLoop:: set_message_callback(ClientMessageCallback callback)
{
    message_callback_ = callback;
}