#pragma once

#include"SelectPoller.h"
#include<functional>
#include"ConnectionManager.h"
#include"Event.h"

    

    using ClientMessageCallback = std::function<void(Connection&,Buffer&)>;
    class EventLoop
    {
    public:
      
        EventLoop(int server_fd);

        void run();

       
        void set_message_callback(ClientMessageCallback callback);

    private:
        void accept_new_connection();
        void handle_client_event(int fd);

        std::vector<Event> get_events();

    private:
        int server_fd_;
        SelectPoller selectPoller_;
        ConnectionManager connectionManager_;
      
        ClientMessageCallback message_callback_;
    };