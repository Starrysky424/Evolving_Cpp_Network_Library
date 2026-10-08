#pragma once

#include <sys/epoll.h>

#include <unordered_map>
#include <vector>
class EpollPoller {
public:
    EpollPoller(int max_events);
    ~EpollPoller();

    void add_fd(int fd);
    void modify_fd(int fd, uint32_t events);
    void remove_fd(int fd);

    int wait(int timeout);

    const std::vector<epoll_event> &get_ready_events() const;

private:
    int epoll_fd_;
    std::vector<epoll_event> ready_events_;

    std::unordered_map<int, uint32_t> fd_events_;
};