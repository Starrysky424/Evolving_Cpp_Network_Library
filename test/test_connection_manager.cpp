#include "ConnectionManager.h"

#include <cassert>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    EpollPoller poller;
    ConnectionManager manager(&poller);

    int sockets1[2];
    int sockets2[2];

    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets1) == 0);
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets2) == 0);

    int fd1 = sockets1[0];
    int fd2 = sockets2[0];

    // 1. 初始状态
    assert(manager.has_connection(fd1) == false);
    assert(manager.get_connection(fd1) == nullptr);

    // 2. 添加连接
    manager.add_connection(fd1);

    assert(manager.has_connection(fd1) == true);
    assert(manager.get_connection(fd1) != nullptr);

    // 3. 添加另一个连接
    manager.add_connection(fd2);

    assert(manager.has_connection(fd1) == true);
    assert(manager.has_connection(fd2) == true);

    assert(manager.get_connection(fd1) != nullptr);
    assert(manager.get_connection(fd2) != nullptr);

    // 4. 删除一个连接
    manager.delete_connection(fd1);

    assert(manager.has_connection(fd1) == false);
    assert(manager.get_connection(fd1) == nullptr);

    // 另一个连接不受影响
    assert(manager.has_connection(fd2) == true);
    assert(manager.get_connection(fd2) != nullptr);

    // 5. 删除不存在的连接
    manager.delete_connection(999);

    assert(manager.has_connection(fd2) == true);

    std::cout << "ConnectionManager tests passed." << std::endl;

    
    close(sockets1[1]);
   
    close(sockets2[1]);

    return 0;
}