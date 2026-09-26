#pragma once
#include<sys/epoll.h>
#include"Buffer.h"
#include "IOEvent.h"
#include"Decoder.h"
#include <chrono>
#include<functional>


class Connection
{

    public:
        using MessageCallback = std::function<void(Connection &, const std::string &)>;

        using CloseCallback = std::function<void(int fd)>;
        Connection(int fd);
        ~Connection();
        // 接收客户端数据
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

       bool enable_write();

        bool disable_write();

        uint32_t events() const;

        //获取最近一次活动时间
        std::chrono::steady_clock::time_point getLastActiveTime() const;

        void setMessageCallback(MessageCallback callback);

        void setCloseCallback(CloseCallback callback);

    private:
        int fd_;
        Buffer input_buffer_;
        uint32_t events_{EPOLLIN};
        Buffer output_buffer_;
        Decoder decoder_;
        CloseCallback close_callback_;
        MessageCallback message_callback_;
        std::chrono::steady_clock::time_point last_active_time_;
};