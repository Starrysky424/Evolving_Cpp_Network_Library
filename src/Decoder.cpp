#include"Decoder.h"

#include<cstdint>
#include<cstring>
#include<arpa/inet.h>

bool Decoder::decode(Buffer &buffer,std::string &message)
{
    if(buffer.read_able_bytes()<4)
        return false;

    uint32_t length = 0;

    std::memcpy(&length, buffer.get(), sizeof(length));

    length = ntohl(length);

    if(buffer.read_able_bytes()<4+length)
    {
        return false;
    }

    message.assign(buffer.get() + 4, length);

    buffer.fetch(4 + length);

    return true;
}