#include"EpollPoller.h"
#include<unistd.h>
#include<stdexcept>
#include"logger.h"
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
        LOG_ERROR(
            "epoll_ctl ADD failed, fd=%d, errno=%d, error=%s",
            fd,
            errno,
            strerror(errno));
        throw std::runtime_error("epoll_ctl add failed");
    }
    fd_events_[fd] = EPOLLIN;
}

void EpollPoller::modify_fd(int fd, uint32_t events)
{

    auto it = fd_events_.find(fd);

    if(it!=fd_events_.end()&&it->second==events)
        return;

    epoll_event event{};

    event.events = events;

    event.data.fd = fd;
    if(epoll_ctl(epoll_fd_,EPOLL_CTL_MOD,fd,&event)==-1)
    {
        throw std::runtime_error("epoll_ctl modfiy failed");
    }
    fd_events_[fd] = events;
}

void EpollPoller:: remove_fd(int fd)
{
    if(epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,fd,nullptr)==-1)
    {
        throw std::runtime_error("epoll_ctl delete failed");
    }

    fd_events_.erase(fd);
}

int EpollPoller:: wait(int timeout)
{

    int n;
    while(true)
    {
        n = epoll_wait(epoll_fd_, ready_events_.data(), ready_events_.size(), timeout);
        if (n < 0)
        {
            if(errno==EINTR)
                continue;
            throw std::runtime_error("epoll_wait failed");
        }
        break;
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