#include"SelectPoller.h"
#include<iostream>
#include<unistd.h>
SelectPoller::SelectPoller()
    : max_fd_(-1)
    
{
    FD_ZERO(&master_fds_);
    FD_ZERO(&ready_fds_);
}

// 加入监听集合
void SelectPoller::add_fd(int fd)
{
    if(fd==-1)
    {
        perror("SelectPoller");
        return;
    }

    FD_SET(fd, &master_fds_);

    if(fd>max_fd_)
        max_fd_ = fd;
}

// 移出监听集合
void SelectPoller:: remove_fd(int fd)
{
    FD_CLR(fd, &master_fds_);
    
    if(fd==max_fd_)
    {
        max_fd_ = -1;
        for (int i = 0; i < FD_SETSIZE;i++)
        {
            if(FD_ISSET(i,&master_fds_))
            {
                max_fd_ = i;
            }
        }
    }
}

int SelectPoller::poll(int timeout_sec)
{
    ready_fds_ = master_fds_;
    struct timeval timeout;
    timeout.tv_sec = timeout_sec;
    timeout.tv_usec = 0;

    int n = select(max_fd_ + 1, &ready_fds_, nullptr, nullptr, &timeout);

    ready_fds_list_.clear();
    if(n<=0)
        return n;

    for (int fd = 0; fd <= max_fd_; fd++)
    {
        if(FD_ISSET(fd,&ready_fds_))
        {
            ready_fds_list_.push_back(fd);
        }
    }
        return n;
}


const std::vector<int> &SelectPoller:: get_ready_fds() const
{
    return ready_fds_list_;
}