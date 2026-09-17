#include<iostream>
#include<signal.h>
#include"EventLoop.h"
#include"ConnectionManager.h"
#include"SocketListener.h"
#include"SelectPoller.h"
int main()
{

    signal(SIGPIPE, SIG_IGN);
    //监听服务器
    SocketListener socketListener("127.0.0.1", 8080);

    EventLoop eventLoop(socketListener.fd());
    eventLoop.set_message_callback(
        [](Connection &connection, const std::string &message)
        {
           
           
            connection.output_buffer().add_data(
               message.data(),
                message.size());

        });
    eventLoop.run();
    return 0;
}