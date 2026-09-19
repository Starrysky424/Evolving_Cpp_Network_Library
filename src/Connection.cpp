#include"Connection.h"
#include<sys/socket.h>
#include<unistd.h>
#include<iostream>
#include <cerrno>
// 构造函数
Connection::Connection(int fd)
    : fd_(fd)
{
}

    IOEvent Connection::recv_data()
    {
        char buffer[1024]{};
        int len = recv(fd_, buffer, sizeof(buffer) - 1, 0);

        if(len>0)
        {
           // std::cout << "recv: " << len << std::endl;
            input_buffer_.add_data(buffer, len);
            return IOEvent::DATA;
        }

        else if(len==0)
        {
            std::cout << "close client" << std::endl;
            return IOEvent::CLOSE;
        }

        if(errno==EAGAIN || errno==EWOULDBLOCK)
        {
            return IOEvent::NONE;
        }
        perror("recv");
        return IOEvent::ERROR;
    }

    bool Connection:: send_data()
    {
        size_t len = output_buffer_.read_able_bytes();

       // std::cout << "try send" << len << std::endl;
        if (len == 0)
            return true;

        ssize_t send_n = send(fd_, output_buffer_.get(), len, 0);

        //std::cout << "actual send:" << send_n << std::endl;
        if (send_n == -1)
        {
            if(errno==EAGAIN||errno==EWOULDBLOCK)
                return true;
            perror("send");
            return false;
        }
        output_buffer_.fetch(send_n);
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

    Buffer &Connection::output_buffer()
    {
        return output_buffer_;
    }

    bool Connection::decode_message(std::string &message)
    {
        return decoder_.decode(input_buffer_, message);
    }
    // 此类的核心思想是将一个客户端连接对应的fd，recv，send，以及连接该数据的Buffer封装到一起，一个Connection就表示一个客户端连接