#include "Connection.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <vector>
// 构造函数
Connection::Connection(int fd, int recv_buffer_size)
    : fd_(fd),
      last_active_time_(std::chrono::steady_clock::now()),
      recv_buffer_size_(recv_buffer_size) {}

Connection::~Connection() {
    ::close(fd_);
}

IOEvent Connection::recv_data() {
    std::vector<char> buffer(recv_buffer_size_);
    bool received = false;
    while (true) {
        ssize_t len = recv(fd_, buffer.data(), buffer.size(), 0);

        if (len > 0) {
            // std::cout << "[Connection] fd=" << fd_
            //           << " recv " << len << " bytes"
            //           << std::endl;
            input_buffer_.add_data(buffer.data(), len);
            last_active_time_ = std::chrono::steady_clock::now();
            received = true;
            continue;
        }

        else if (len == 0) {
            std::cout << "close client" << std::endl;

            return IOEvent::CLOSE;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        }

        perror("recv");

        return IOEvent::ERROR;
    }

    if (received) {
        if (message_callback_) {
            std::string message;

            while (decode_message(message)) {
                message_callback_(*this, message);

                message.clear();
            }
        }
        return IOEvent::DATA;
    }

    return IOEvent::NONE;
}

bool Connection::send_data() {
    while (output_buffer_.read_able_bytes() > 0) {
        ssize_t send_n = send(fd_, output_buffer_.get(), output_buffer_.read_able_bytes(), 0);

        if (send_n > 0) {
            output_buffer_.fetch(send_n);
            last_active_time_ = std::chrono::steady_clock::now();
            continue;
        }

        if (send_n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (enable_write_callback_) {
                    enable_write_callback_();
                }
                return true;
            }

            if (errno == EINTR)
                continue;

            perror("send");
            return false;
        }
    }

    if (disable_write_callback_) {
        disable_write_callback_();
    }
    return true;
}

int Connection::fd() const {
    return fd_;
}

// 获取输入Buffer
Buffer &Connection::input_buffer() {
    return input_buffer_;
}

Buffer &Connection::output_buffer() {
    return output_buffer_;
}

bool Connection::decode_message(std::string &message) {
    if (!decoder_)
        return false;

    auto result = decoder_->decode(input_buffer_);

    if (!result)
        return false;

    message = std::move(*result);
    return true;
}

bool Connection::enable_write() {
    if (!(events_ & EPOLLOUT)) {
        events_ |= EPOLLOUT;
        return true;
    }
    return false;
}

uint32_t Connection::events() const {
    return events_;
}

bool Connection::disable_write() {
    if (events_ & EPOLLOUT) {
        events_ &= ~EPOLLOUT;
        return true;
    }
    return false;
}

std::chrono::steady_clock::time_point Connection::getLastActiveTime() const {
    return last_active_time_;
}

void Connection::setMessageCallback(MessageCallback callback) {
    message_callback_ = std::move(callback);
}

void Connection::setCloseCallback(CloseCallback callback) {
    close_callback_ = std::move(callback);
}

void Connection::setWriteCallbacks(std::function<void()> enable, std::function<void()> disable) {
    enable_write_callback_ = std::move(enable);
    disable_write_callback_ = std::move(disable);
}

void Connection::setDecoder(std::unique_ptr<FrameDecoder> decoder) {
    decoder_ = std::move(decoder);
}
// 此类的核心思想是将一个客户端连接对应的fd，recv，send，以及连接该数据的Buffer封装到一起，一个Connection就表示一个客户端连接