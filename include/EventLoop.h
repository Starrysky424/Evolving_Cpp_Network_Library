#pragma once

#include"SelectPoller.h"

#include"ConnectionManager.h"

class EventLoop
{
    public:
        EventLoop(int server_fd);

        void run();

    private:
        void accept_new_connection();
        void handle_client_event(int fd);

    private:
        int server_fd_;
        SelectPoller selectPoller_;
        ConnectionManager connectionManager_;

};