#pragma once

#include"EpollPoller.h"
#include<functional>
#include"ConnectionManager.h"
#include"Event.h"
#include<string>
    

    using ClientMessageCallback = std::function<void(Connection&,const std::string&)>;
    class EventLoop
    {
    public:
      
        EventLoop(int server_fd);

        void run();

       
        void set_message_callback(ClientMessageCallback callback);

    private:
        void accept_new_connection();
        void handle_client_event(int fd);

        std::vector<Event> get_events(int n);

    private:
        int server_fd_;
        EpollPoller epollPoller_;
        ConnectionManager connectionManager_;

        ClientMessageCallback message_callback_;
    };