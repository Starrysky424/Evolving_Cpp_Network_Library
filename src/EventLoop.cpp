#include"EventLoop.h"
#include<iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
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

        bool check = client->recv_data();
        if (!check)
        {
            connectionManager_.delete_connection(fd);
            selectPoller_.remove_fd(fd);
            close(fd);
            return;
        }

        std::cout << "success recv" << std::endl;
        client->send_data();
    }

void EventLoop::run()
{
    while (true)
    {
        int n = selectPoller_.poll(5);
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

        for (int fd : selectPoller_.get_ready_fds())
        {
            
            if (fd == server_fd_)
            {
                accept_new_connection();
            }

            // 客户端传输数据
            else
            {
                handle_client_event(fd);
                        }
        }
    }
    
}