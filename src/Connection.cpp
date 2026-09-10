#include"Connection.h"
#include<sys/socket.h>
#include<unistd.h>
#include<iostream>
    //构造函数
    Connection::Connection(int fd)
        : fd_(fd)
        {}

    bool Connection::recv_data()
    {
        char buffer[1024]{};
        int len = recv(fd_, buffer, sizeof(buffer) - 1, 0);

        if(len>0)
        {
            input_buffer_.add_data(buffer, len);
            return true;
        }

        else if(len==0)
        {
            std::cout << "close client" << std::endl;
            return false;
        }

        perror("recv");
        return false;
    }

    bool Connection:: send_data()
    {
        size_t len = input_buffer_.read_able_bytes();
        if(len==0)
            return true;
        int send_n = send(fd_, input_buffer_.get(), len, 0);

        if(send_n==-1)
        {
            perror("send");
            return false;
        }
        input_buffer_.fetch(send_n);
        return true;
    }

    int Connection::fd() const
    {
        return fd_;
    }

    // 获取输入Buffer
    Buffer &Connection::input_buffer()
    {
        return input_buffer_;
    }

   

    // 此类的核心思想是将一个客户端连接对应的fd，recv，send，以及连接该数据的Buffer封装到一起，一个Connection就表示一个客户端连接