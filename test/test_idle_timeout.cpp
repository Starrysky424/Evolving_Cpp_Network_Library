#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    std::cout << "连接成功，开始空闲等待 70 秒..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(70));

    char buffer[1024];
    ssize_t n = recv(sockfd, buffer, sizeof(buffer), 0);

    if (n == 0)
    {
        std::cout << "服务器已关闭连接，Idle Timeout 测试通过"
                  << std::endl;
    }
    else if (n == -1)
    {
        perror("recv");
    }
    else
    {
        std::cout << "收到服务器数据：" << n << " 字节" << std::endl;
    }

    close(sockfd);
    return 0;
}