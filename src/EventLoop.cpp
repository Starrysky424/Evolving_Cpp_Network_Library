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
        epollPoller_.add_fd(server_fd_);
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

        epollPoller_.add_fd(client_fd);
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
            // std::cout << "server close fd: " << fd
            //           << ", event: " << static_cast<int>(event)
            //           << std::endl;
            connectionManager_.delete_connection(fd);
            epollPoller_.remove_fd(fd);
            close(fd);
            
        }

        else  if(event==IOEvent::DATA)
        {
           
            if (message_callback_)
            {
                std::string message;

                while (client->decode_message(message))
                {
                    message_callback_(*client,
                                      message);

                    message.clear();
                }
                
               
            }

            if(client->output_buffer().read_able_bytes()>0)
            {
               if(client->enable_write()){
                   epollPoller_.modify_fd(fd, client->events());
               }

              
            }
           
        }
    }

void EventLoop::run()
{
    
    while (true)
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

            else if(event.type==EventType::WRITE)
            {
                Connection *client = connectionManager_.get_connection(event.fd);
                
                if(client==nullptr)
                    continue;

                client->send_data();

                if(client->output_buffer().read_able_bytes()==0)
                {
                    if(client->disable_write())
                    {
                        epollPoller_.modify_fd(event.fd, client->events());
                    }
                    
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