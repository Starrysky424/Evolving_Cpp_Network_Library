#include"SocketListener.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<iostream>
#include<fcntl.h>
SocketListener::SocketListener(const char *ip, int port)
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(fd_==-1)
    {
        perror("socket");
        return;
    }

    //设置监听Socket非阻塞
    int flags = fcntl(fd_, F_GETFL, 0);

    if(flags==-1)
    {
        perror("fcntl");
        close(fd_);
        fd_ = -1;
        return;
    }

    if(fcntl(fd_,F_SETFL,flags | O_NONBLOCK)==-1)
    {
        perror("fcntl");
        close(fd_);
        fd_ = -1;
        return;
    }
    
    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(ip);

    server_addr.sin_port = htons(port);

    if(bind(fd_,(sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        perror("bind");
        close(fd_);
        fd_ = -1;
        return;
    }

    if(listen(fd_,10)==-1)
    {
        perror("listen");
        close(fd_);
        fd_ = -1;
        return;
    }

    std::cout << "server listen on " << ip << ":" << port << std::endl;
}

int SocketListener::fd()const
{
    return fd_;
}