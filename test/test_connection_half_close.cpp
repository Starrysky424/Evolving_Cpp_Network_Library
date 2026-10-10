
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
        std::cerr << "Invalid server address\n";
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    const std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Connection: close\r\n"
        "\r\n";

    size_t sent = 0;
    while (sent < request.size()) {
        ssize_t n = send(sockfd, request.data() + sent, request.size() - sent, 0);

        if (n > 0) {
            sent += static_cast<size_t>(n);
        } else if (n == -1 && errno == EINTR) {
            continue;
        } else {
            perror("send");
            close(sockfd);
            return 1;
        }
    }

    // 只关闭客户端发送方向，仍然保留接收响应的能力。
    if (shutdown(sockfd, SHUT_WR) == -1) {
        perror("shutdown");
        close(sockfd);
        return 1;
    }

    std::string response;
    std::array<char, 1024> buffer{};

    while (true) {
        ssize_t n = recv(sockfd, buffer.data(), buffer.size(), 0);

        if (n > 0) {
            response.append(buffer.data(), static_cast<size_t>(n));
        } else if (n == 0) {
            break;
        } else if (errno == EINTR) {
            continue;
        } else {
            perror("recv");
            close(sockfd);
            return 1;
        }
    }

    close(sockfd);

    if (response.find("HTTP/1.1 200 OK\r\n") != 0) {
        std::cerr << "Unexpected response:\n" << response << '\n';
        return 1;
    }

    if (response.find("hello,world\n") == std::string::npos) {
        std::cerr << "Response body is missing\n";
        return 1;
    }

    std::cout << "Connection half-close test passed\n";
    return 0;
}
