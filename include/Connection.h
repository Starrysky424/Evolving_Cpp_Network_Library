#pragma once

#include"Buffer.h"

class Connection
{

    public:
        Connection(int fd);
        //接收客户端数据
        bool recv_data();

        //向客户端发送数据
        bool send_data();

        //获取客户端fd
        int fd() const;

        //获取输入Buffer
        Buffer &input_buffer();

        
    private:
        int fd_;
        Buffer input_buffer_;
};