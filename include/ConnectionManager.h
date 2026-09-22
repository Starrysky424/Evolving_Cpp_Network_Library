#pragma once

#include "Connection.h"
#include<unordered_map>

class ConnectionManager
{
    public:
        //添加连接
        void add_connection(int fd);

        //删除连接
        void delete_connection(int fd);

        //根据fd获取Connection
        Connection *get_connection(int fd);

        bool has_connection(int fd) const;


        
    private:
        std::unordered_map<int, Connection> connections_;
};