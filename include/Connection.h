#pragma once

#include"Buffer.h"
#include "IOEvent.h"

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

    private:
        int fd_;
        Buffer input_buffer_;

        Buffer output_buffer_;
};