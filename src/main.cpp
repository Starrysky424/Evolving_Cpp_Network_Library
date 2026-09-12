#include<iostream>

#include"EventLoop.h"
#include"ConnectionManager.h"
#include"SocketListener.h"
#include"SelectPoller.h"
int main()
{
    //监听服务器
    SocketListener socketListener("127.0.0.1", 8080);

    EventLoop eventLoop(socketListener.fd());
    
    eventLoop.run();
    return 0;
}