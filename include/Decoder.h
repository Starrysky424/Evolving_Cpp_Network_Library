#pragma once 

#include"Buffer.h"
#include<string>

class Decoder
{
    public:
        bool decode(Buffer &buffer, std::string &message);
};