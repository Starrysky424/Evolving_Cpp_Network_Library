#pragma once
#include<sys/epoll.h>
#include"Buffer.h"
#include "IOEvent.h"
#include"Decoder.h"
class Connection
{

    public:
        Connection(int fd);
        //接收客户端数据
        IOEvent recv_data();

        //向客户端发送数据
       bool send_data();

        //获取客户端fd
        int fd() const;

        //获取输入Buffer
        Buffer &input_buffer();

        //获取输出的Buffer
        Buffer &output_buffer();

        //解析一条完整通知
        bool decode_message(std::string &message);

        void enable_write();

        void disable_write();

    private:
        int fd_;
        Buffer input_buffer_;
        uint32_t events_{EPOLLIN};
        Buffer output_buffer_;
        Decoder decoder_;
};