#include "Buffer.h"

#include <cassert>
#include <cstring>
#include <iostream>

int main()
{
    Buffer buffer;

    // 1. 初始状态
    assert(buffer.read_able_bytes() == 0);

    // 2. 添加数据
    const char *hello = "hello";
    buffer.add_data(hello, 5);

    assert(buffer.read_able_bytes() == 5);
    assert(std::memcmp(buffer.get(), "hello", 5) == 0);

    // 3. peek
    assert(*buffer.peek(0) == 'h');
    assert(*buffer.peek(4) == 'o');
    assert(buffer.peek(5) == nullptr);

    // 4. 部分消费
    buffer.fetch(2);

    assert(buffer.read_able_bytes() == 3);
    assert(std::memcmp(buffer.get(), "llo", 3) == 0);

    // 5. 继续追加数据
    const char *world = " world";
    buffer.add_data(world, 6);

    assert(buffer.read_able_bytes() == 9);
    assert(std::memcmp(buffer.get(), "llo world", 9) == 0);

    // 6. fetch 超过剩余长度
    buffer.fetch(100);

    assert(buffer.read_able_bytes() == 0);

    // 7. 空数据
    buffer.add_data(nullptr, 0);

    assert(buffer.read_able_bytes() == 0);

    std::cout << "Buffer tests passed." << std::endl;

    return 0;
}