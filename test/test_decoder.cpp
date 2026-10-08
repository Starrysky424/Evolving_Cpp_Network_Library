#include "Buffer.h"
#include "decoder/Decoder.h"

#include <arpa/inet.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

std::vector<char> make_packet(const std::string &message) {
    uint32_t length = htonl(static_cast<uint32_t>(message.size()));

    std::vector<char> packet(4 + message.size());

    std::memcpy(packet.data(), &length, 4);
    std::memcpy(packet.data() + 4, message.data(), message.size());

    return packet;
}

int main() {
    Decoder decoder;

    // =========================
    // 1. 完整包测试
    // =========================
    {
        Buffer buffer;

        auto packet = make_packet("hello");
        buffer.add_data(packet.data(), packet.size());

        auto result = decoder.decode(buffer);

        assert(result.has_value());
        assert(*result == "hello");
        assert(buffer.read_able_bytes() == 0);
    }

    // =========================
    // 2. 半包测试
    // =========================
    {
        Buffer buffer;

        auto packet = make_packet("hello");
        buffer.add_data(packet.data(), 6);

        auto result = decoder.decode(buffer);

        assert(!result.has_value());

        // 半包不能被消费
        assert(buffer.read_able_bytes() == 6);
    }

    // =========================
    // 3. 粘包测试
    // =========================
    {
        Buffer buffer;

        auto packet1 = make_packet("hello");
        auto packet2 = make_packet("world");

        // 一次 recv 收到两个完整消息
        buffer.add_data(packet1.data(), packet1.size());
        buffer.add_data(packet2.data(), packet2.size());

        // 第一次解析
        auto result = decoder.decode(buffer);

        assert(result.has_value());
        assert(*result == "hello");

        // 第二次解析
        result = decoder.decode(buffer);

        assert(result.has_value());
        assert(*result == "world");

        // 两个消息都被消费
        assert(buffer.read_able_bytes() == 0);
    }

    std::cout << "Decoder tests passed." << std::endl;

    return 0;
}