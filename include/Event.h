#pragma once

enum class EventType
{
    NEW_CONNECTION, //建立新连接

    READ, //读事件

    WRITE, //写事件

    CLOSE  //关闭
};

struct Event
{
    int fd;

    EventType type;

    Event(int fd, EventType type)
        : fd(fd),
          type(type)
          {

          }
};