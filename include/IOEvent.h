#pragma once

enum class IOEvent
{
    DATA,  //收到数据

    CLOSE,  //关闭客户端

    ERROR   //网络错误
};