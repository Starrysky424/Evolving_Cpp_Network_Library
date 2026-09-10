#pragma once

#include <vector>
#include <string>

class Buffer
{
public:
    // 添加数据
    void add_data(const char *data, size_t len);

    // 当前可读数据大小
    size_t read_able_bytes() const;

    // 获取可读数据的开始位置指针
    const char *get() const;

    // 移动len个位置，表示已读
    void fetch(size_t len);

private:
    std::vector<char> buffer_;
    size_t read_index_ = 0;
};

