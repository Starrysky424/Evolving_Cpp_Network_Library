#pragma once

#include "Connection.h"
#include<unordered_map>
#include<functional>
#include"EpollPoller.h"
class ConnectionManager
{
    public:
        explicit ConnectionManager(EpollPoller *poller);
        // 添加连接
        void add_connection(int fd);

        //删除连接
        void delete_connection(int fd);

        //根据fd获取Connection
        Connection *get_connection(int fd);

        bool has_connection(int fd) const;

        void forEachConn(const std::function<void(Connection *)> &callback);

    private:
        std::unordered_map<int, Connection> connections_;

        EpollPoller *poller_;
};