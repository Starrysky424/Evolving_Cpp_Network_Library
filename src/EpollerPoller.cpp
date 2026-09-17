#include"EpollPoller.h"
#include<unistd.h>
#include<stdexcept>
EpollPoller::EpollPoller()
{
    epoll_fd_ = epoll_create1(0);
    if(epoll_fd_==-1)
    {
        throw std::runtime_error("epoll_create1 failed");
    }
    ready_events_.resize(1024);
}

void EpollPoller::add_fd(int fd)
{
    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = fd;
    if(epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,fd,&event)==-1)
    {
        throw std::runtime_error("epoll_ctl add failed");
    }
}
void EpollPoller:: remove_fd(int fd)
{
    if(epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,fd,nullptr)==-1)
    {
        throw std::runtime_error("epoll_ctl delete failed");
    }
}
int EpollPoller:: wait(int timeout)
{
    int n = epoll_wait(epoll_fd_, ready_events_.data(), ready_events_.size(), timeout);

    if(n==-1)
    {
        throw std::runtime_error("epoll_wait failed");

    }
    return n;
}

const std::vector<epoll_event> &EpollPoller:: get_ready_events() const
{
    return ready_events_;
}

 EpollPoller:: ~EpollPoller()
{
    close(epoll_fd_);
}