#include "decoder/DelimiterDecoder.h"

#include <iostream>
#include <string_view>
#include <utility>

DelimiterDecoder::DelimiterDecoder(std::string delimiter) : delimiter_(std::move(delimiter)) {}

std::optional<std::string> DelimiterDecoder::decode(Buffer &buffer) {
    const char *data = buffer.get();
    const size_t length = buffer.read_able_bytes();

    // std::cout << "[DelimiterDecoder] readable bytes: "
    //           << length << std::endl;

    if (length == 0) {
        return std::nullopt;
    }

    std::string_view view(data, length);

    const size_t pos = view.find(delimiter_);

    if (pos == std::string_view::npos) {
        // std::cout << "[DelimiterDecoder] delimiter not found"
        //           << std::endl;

        return std::nullopt;
    }

    const size_t message_length = pos + delimiter_.size();

    // std::cout << "[DelimiterDecoder] message length: "
    //           << message_length << std::endl;

    std::string message(data, message_length);

    buffer.fetch(message_length);

    return message;
}

std::unique_ptr<FrameDecoder> DelimiterDecoder::clone() const {
    return std::make_unique<DelimiterDecoder>(delimiter_);
}