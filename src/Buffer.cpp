#include"Buffer.h"
    //往buffer里加数据
    void Buffer::add_data(const char *data, size_t len)

    {
    if(data==nullptr||len==0)
        return;

    buffer_.insert(buffer_.end(), data, data + len);

    }


    //read_index移动len个位置
    void Buffer::fetch(size_t len)
    {
        if (len > read_able_bytes())
            len = read_able_bytes();
        read_index_ += len;
    }

    size_t Buffer::read_able_bytes()const
    {
        
        return buffer_.size() - read_index_;
    }


    const char* Buffer::get()const
    {
        return buffer_.data() + read_index_;
    }

    // 核心实现并不是每次读取数据后从头删除数据 而是定义一个read_index指针表示已读数据的位置，好处就是避免频繁移动和拷贝剩余数据的情况，读写可以解耦，提高 Buffer 的读写效率，同时更适合网络数据分批到达的场景