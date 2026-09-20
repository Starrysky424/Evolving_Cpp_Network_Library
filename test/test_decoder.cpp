#include"Buffer.h"
#include"Decoder.h"

#include <arpa/inet.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

std::vector<char>make_packet(const std::string& message)
{
    uint32_t length = htonl(static_cast<uint32_t>(message.size()));

    std::vector<char> packet(4 + message.size());
    std::memcpy(packet.data(), &length, 4);
    std::memcpy(packet.data() + 4, message.data(), message.size());
    return packet;
}
int main()
{
    Decoder decoder;

    // =========================
    // 1. 完整包测试
    // =========================
    {Buffer buffer;
   
    std::string message;

    auto packet = make_packet("hello");
    buffer.add_data(packet.data(), packet.size());

    bool result = decoder.decode(buffer, message);

    assert(result == true);
    assert(message == "hello");
    assert(buffer.read_able_bytes() == 0);
    }

    // =========================
    // 2. 半包测试
    // =========================

    {
        Buffer buffer;
        std::string message;
        auto packet = make_packet("hello");
        buffer.add_data(packet.data(), 6);

        bool result = decoder.decode(buffer, message);

        assert(result == false);

        // 半包不能被消费
        assert(buffer.read_able_bytes() == 6);
    }

    // =========================
    // 3. 粘包测试
    // =========================
    {
        Buffer buffer;
        std::string message;

        auto packet1 = make_packet("hello");
        auto packet2 = make_packet("world");

        // 一次 recv 收到两个完整消息
        buffer.add_data(packet1.data(), packet1.size());
        buffer.add_data(packet2.data(), packet2.size());

        // 第一次解析
        bool result = decoder.decode(buffer, message);

        assert(result == true);
        assert(message == "hello");

        // 第二次解析
        result = decoder.decode(buffer, message);

        assert(result == true);
        assert(message == "world");

        // 两个消息都被消费
        assert(buffer.read_able_bytes() == 0);
    }

    std::cout << "Decoder tests passed." << std::endl;
    return 0;
}