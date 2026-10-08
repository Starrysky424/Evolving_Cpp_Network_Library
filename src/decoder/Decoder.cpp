#include "decoder/Decoder.h"

#include <arpa/inet.h>

#include <cstdint>
#include <cstring>

std::optional<std::string> Decoder::decode(Buffer &buffer) {
    if (buffer.read_able_bytes() < 4)
        return std::nullopt;

    uint32_t length = 0;

    std::memcpy(&length, buffer.get(), sizeof(length));

    length = ntohl(length);

    if (buffer.read_able_bytes() < 4 + length)
        return std::nullopt;

    std::string message(buffer.get() + 4, length);

    buffer.fetch(4 + length);
    return message;
}

std::unique_ptr<FrameDecoder> Decoder::clone() const {
    return std::make_unique<Decoder>();
}
