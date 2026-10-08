#include "SocketListener.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
SocketListener::SocketListener(Config &config) : fd_(-1) {
    initSocket(config.port, config.backlog);
}

SocketListener::~SocketListener() {
    close();
}

int SocketListener::fd() const {
    return fd_;
}

void SocketListener::initSocket(int port, int backlog) {
    fd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (fd_ == -1) {
        throw std::runtime_error("socket creation failed");
    }

    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) {
        close();
        throw std::runtime_error("fcntl F_GETFL failed");
    }

    if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        close();
        throw std::runtime_error("fcntl F_SETFL failed");
    }

    int opt = 1;

    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close();
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");
    }

    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        close();
        throw std::runtime_error("setsockopt SO_REUSEPORT failed");
    }

    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(fd_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
        close();
        throw std::runtime_error("bind failed");
    }

    if (listen(fd_, backlog) == -1) {
        close();
        throw std::runtime_error("listen failed");
    }

    std::cout << "server listen on 0.0.0.0:" << port << std::endl;
}

int SocketListener::accept() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd = ::accept(fd_, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

    if (client_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -1;
        }

        throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
    }

    int flags = fcntl(client_fd, F_GETFL, 0);

    if (flags == -1) {
        ::close(client_fd);
        throw std::runtime_error("fcntl client fd failed");
    }

    if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        ::close(client_fd);
        throw std::runtime_error("set client fd nonblocking failed");
    }

    char client_ip[INET_ADDRSTRLEN]{};

    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    std::cout << "new client:" << client_ip << ":" << ntohs(client_addr.sin_port)
              << " fd:" << client_fd << std::endl;

    return client_fd;
}

void SocketListener::close() {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}