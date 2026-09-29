#pragma once

#include <string>

#define CONFIG_ITEMS(X)                                          \
    X(port, "port", "--port", 8080)                              \
    X(thread_count, "threads", "--threads", 4)                   \
    X(backlog, "backlog", "--backlog", 128)                      \
    X(max_events, "max-events", "--max-events", 4096)            \
    X(epoll_timeout_ms, "epoll-timeout", "--epoll-timeout", 5000) \
    X(recv_buffer_size, "recv-buffer", "--recv-buffer", 65536)

    struct Config
{
#define X(field, json_key, cli_flag, default_val) int field = default_val;
    CONFIG_ITEMS(X)
#undef X

    std::string config_path = "config.json";
    bool dump_config_ = false;

    void init(int argc, char **argv);

    void loadFromFile(const std::string &path); // 读取配置文件并覆盖默认值

    void parseCommandLine(int argc, char **argv); // 解析启动程序时输入的命令行参数并覆盖默认值

    void print() const; // 把最终配置打印出来
};
