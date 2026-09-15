#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

const char *IP = "127.0.0.1";
const int PORT = 8080;


void client_task(int id,int request_count)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return;
    }
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET,
              IP,
              &server.sin_addr);
    if (connect(fd,
                (sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("connect");
        return;
    }
    const char *msg = "hello";
    char buffer[1024];
    
    
    for (int i = 0; i < request_count; i++)
    {
        send(fd,
             msg,
             strlen(msg),
             0);
        recv(fd,
             buffer,
             sizeof(buffer),
             0);
    }
    close(fd);
}
int main()
{
    // 模拟客户端数量
    int client_num = 100;

    // 每个客户端请求次数
    int request_per_client = 1000;

    std::vector<std::thread> threads;

    auto start =
        std::chrono::steady_clock::now();

    // 创建10个客户端
    for (int i = 0; i < client_num; i++)
    {
        threads.emplace_back(
            client_task,
            i,
            request_per_client);
    }

    // 等待所有客户端结束
    for (auto &t : threads)
    {
        t.join();
    }

    auto end =
        std::chrono::steady_clock::now();

    auto cost =
        std::chrono::duration_cast<
            std::chrono::milliseconds>(end - start)
            .count();

    int total_request =
        client_num * request_per_client;

    std::cout
        << "clients: "
        << client_num
        << std::endl;

    std::cout
        << "request: "
        << total_request
        << std::endl;

    std::cout
        << "time: "
        << cost
        << " ms"
        << std::endl;

    std::cout
        << "QPS: "
        << total_request / (cost / 1000.0)
        << std::endl;

    return 0;
}
