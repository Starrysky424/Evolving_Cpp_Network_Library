#include "ConnectionManager.h"

#include <cassert>
#include <iostream>

int main()
{
    EpollPoller poller;
    ConnectionManager manager(&poller);

    // 1. 初始状态
    assert(manager.has_connection(100) == false);
    assert(manager.get_connection(100) == nullptr);

    // 2. 添加连接
    manager.add_connection(100);

    assert(manager.has_connection(100) == true);
    assert(manager.get_connection(100) != nullptr);

    // 3. 添加另一个连接
    manager.add_connection(200);

    assert(manager.has_connection(100) == true);
    assert(manager.has_connection(200) == true);

    assert(manager.get_connection(100) != nullptr);
    assert(manager.get_connection(200) != nullptr);

    // 4. 删除一个连接
    manager.delete_connection(100);

    assert(manager.has_connection(100) == false);
    assert(manager.get_connection(100) == nullptr);

    // 另一个连接不受影响
    assert(manager.has_connection(200) == true);
    assert(manager.get_connection(200) != nullptr);

    // 5. 删除不存在的连接
    manager.delete_connection(999);

    assert(manager.has_connection(200) == true);

    std::cout << "ConnectionManager tests passed." << std::endl;

    return 0;
}