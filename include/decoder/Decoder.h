#pragma once 

#include"Buffer.h"
#include<string>
#include"decoder/FrameDecoder.h"
class Decoder: public FrameDecoder
{
    public:
        std::optional<std::string> decode(Buffer &buffer) override;

        std::unique_ptr<FrameDecoder> clone() const override;

       
};