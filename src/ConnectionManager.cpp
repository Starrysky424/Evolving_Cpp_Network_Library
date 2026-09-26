#include"ConnectionManager.h"
#include<unistd.h>
ConnectionManager::ConnectionManager(EpollPoller *poller)
    : poller_(poller)
{
}

    void ConnectionManager::add_connection(int fd)
    {

        auto result = connections_.try_emplace(fd, fd);

        if(result.second&&poller_)
        {
            poller_->add_fd(fd);
        }
    }

    void ConnectionManager::delete_connection(int fd)
    {
        if(poller_)
            poller_->remove_fd(fd);

        connections_.erase(fd);

    }

    Connection *ConnectionManager::get_connection(int fd)
    {
        auto it = connections_.find(fd);
        if(it==connections_.end())
            return nullptr;
        return &it->second;
    }

    bool ConnectionManager::has_connection(int fd)const
    {
        return connections_.find(fd) != connections_.end();
    }

    void ConnectionManager:: forEachConn(const std::function<void(Connection *)> &callback)
    {
        for(auto &pair : connections_)
        {
            callback(&pair.second);
        }
    }