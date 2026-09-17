#pragma once

#include<sys/epoll.h>
#include<vector>

class EpollPoller
{
    public:
        EpollPoller();
        ~EpollPoller();

        void add_fd(int fd);
        void modify_fd(int fd, uint32_t events);
        void remove_fd(int fd);

        int wait(int timeout);

        const std::vector<epoll_event> &get_ready_events() const;

    private:
        int epoll_fd_;
        std::vector<epoll_event> ready_events_;
};