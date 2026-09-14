#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET,
              "127.0.0.1",
              &server.sin_addr);
    if (connect(fd,
                (sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("connect");
        return -1;
    }
    std::cout << "connect success\n";
    const char *msg = "hello";
    char buffer[1024];
    int count = 10000;
    auto start =
        std::chrono::steady_clock::now();
    for (int i = 0; i < count; i++)
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
    auto end =
        std::chrono::steady_clock::now();
    auto cost =
        std::chrono::duration_cast<
            std::chrono::milliseconds>(end - start)
            .count();
    std::cout << "request: "
              << count
              << std::endl;
    std::cout << "time: "
              << cost
              << " ms\n";
    std::cout << "QPS: "
              << count / (cost / 1000.0)
              << std::endl;
    close(fd);
}
