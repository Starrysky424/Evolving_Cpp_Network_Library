#pragma once

#include "decoder/FrameDecoder.h"

#include <string>

class DelimiterDecoder : public FrameDecoder {
public:
    explicit DelimiterDecoder(std::string delimiter);

    std::optional<std::string> decode(Buffer &buffer) override;

    std::unique_ptr<FrameDecoder> clone() const override;

private:
    std::string delimiter_;
};