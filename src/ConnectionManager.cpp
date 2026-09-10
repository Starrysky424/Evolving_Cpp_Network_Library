#include"ConnectionManager.h"

    void ConnectionManager::add_connection(int fd)
    {

        connections_.emplace(fd, Connection(fd));
    }

    void ConnectionManager::delete_connection(int fd)
    {
        
        connections_.erase(fd);
        
    }

    Connection *ConnectionManager::get_connection(int fd)
    {
        auto it = connections_.find(fd);
        if(it==connections_.end())
            return nullptr;
        return &it->second;
    }