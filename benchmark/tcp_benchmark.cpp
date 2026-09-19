#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <string>
#include <cerrno>

#include "Buffer.h"
#include "Decoder.h"

//一个客户端线程的测试结果
struct WorkerResult
{
    uint64_t completed = 0;
    uint64_t errors = 0;
    std::vector<long long> latency_us;
};

//控制所有客户端同时开始
struct StartGate
{
    std::mutex mutex;
    std::condition_variable cv;
    int ready = 0;
    bool start = false;
};

// 保证一次 send 完整发送
static bool send_all(int fd, const void *data, size_t len)
{
    const char *ptr = static_cast<const char *>(data);

    while (len > 0)
    {
        ssize_t n = send(fd, ptr, len, MSG_NOSIGNAL);

        if (n > 0)
        {
            ptr += n;
            len -= static_cast<size_t>(n);
            continue;
        }

        if (n < 0 && errno == EINTR)
            continue;

        return false;
    }

    return true;
}

// 发送：4字节长度 + 消息内容
static bool send_message(int fd, const std::string &message)
{
    uint32_t length = htonl(static_cast<uint32_t>(message.size()));

    std::vector<char> packet(sizeof(length) + message.size());

    std::memcpy(packet.data(),
                &length,
                sizeof(length));

    std::memcpy(packet.data() + sizeof(length),
                message.data(),
                message.size());

    return send_all(fd,
                    packet.data(),
                    packet.size());
}

// 接收完整的一条消息
static bool receive_message(
    int fd,
    Buffer &input_buffer,
    Decoder &decoder,
    std::string &message)
{
    while (true)
    {
        // Buffer 中已经有完整消息
        if (decoder.decode(input_buffer, message))
            return true;

        char buffer[4096];

        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);

        if (n > 0)
        {
            input_buffer.add_data(
                buffer,
                static_cast<size_t>(n));

            continue;
        }

        if (n < 0 && errno == EINTR)
            continue;

        return false;
    }
}

// 建立 TCP 连接
static int connect_server()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1)
    {
        perror("socket");
        return -1;
    }

    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    if (inet_pton(
            AF_INET,
            "127.0.0.1",
            &server_addr.sin_addr) != 1)
    {
        std::cerr << "invalid server address\n";
        close(fd);
        return -1;
    }

    if (connect(
            fd,
            reinterpret_cast<sockaddr *>(&server_addr),
            sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
}

// 一个连接对应一个 worker
static void worker(
    int fd,
    int requests,
    const std::string &message,
    StartGate &gate,
    WorkerResult &result)
{
    // 等待所有连接准备好
    {
        std::unique_lock<std::mutex> lock(gate.mutex);

        ++gate.ready;

        gate.cv.notify_all();

        gate.cv.wait(lock, [&gate]
                     { return gate.start; });
    }

    result.latency_us.reserve(
        static_cast<size_t>(requests));

    Buffer input_buffer;
    Decoder decoder;

    for (int i = 0; i < requests; ++i)
    {
        auto begin =
            std::chrono::steady_clock::now();

        // 发送请求
        if (!send_message(fd, message))
        {
            ++result.errors;
            break;
        }

        // 接收响应
        std::string response;

        if (!receive_message(
                fd,
                input_buffer,
                decoder,
                response))
        {
            ++result.errors;
            break;
        }

        auto end =
            std::chrono::steady_clock::now();

        // 检查服务器是否正确 echo
        if (response != message)
        {
            ++result.errors;
            break;
        }

        auto latency =
            std::chrono::duration_cast<
                std::chrono::microseconds>(end - begin)
                .count();

        result.latency_us.push_back(latency);

        ++result.completed;
    }

    close(fd);
}

// 计算 P50 / P99
static long long percentile(
    std::vector<long long> &values,
    double p)
{
    if (values.empty())
        return 0;

    std::sort(values.begin(), values.end());

    size_t index =
        static_cast<size_t>(
            p * static_cast<double>(values.size()));

    if (index == 0)
        index = 1;

    --index;

    if (index >= values.size())
        index = values.size() - 1;

    return values[index];
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr
            << "Usage: "
            << argv[0]
            << " <connections> "
            << "<requests_per_connection> "
            << "<message_size>\n";

        std::cerr
            << "Example: "
            << argv[0]
            << " 100 1000 1024\n";

        return 1;
    }

    int connections =
        std::atoi(argv[1]);

    int requests_per_connection =
        std::atoi(argv[2]);

    int message_size =
        std::atoi(argv[3]);

    if (connections <= 0 ||
        requests_per_connection <= 0 ||
        message_size < 0)
    {
        std::cerr
            << "invalid benchmark arguments\n";

        return 1;
    }

    // 建立多个 TCP 连接
    std::vector<int> fds;

    fds.reserve(
        static_cast<size_t>(connections));

    for (int i = 0; i < connections; ++i)
    {
        int fd = connect_server();

        if (fd == -1)
        {
            for (int opened_fd : fds)
                close(opened_fd);

            std::cerr
                << "failed to create all connections: "
                << i << "/"
                << connections
                << "\n";

            return 1;
        }

        fds.push_back(fd);
    }

    // 所有请求发送相同大小的消息
    std::string message(
        static_cast<size_t>(message_size),
        'A');

    StartGate gate;

    std::vector<WorkerResult> results(
        static_cast<size_t>(connections));

    std::vector<std::thread> workers;

    workers.reserve(
        static_cast<size_t>(connections));

    // 创建 worker
    for (int i = 0; i < connections; ++i)
    {
        workers.emplace_back(
            worker,
            fds[static_cast<size_t>(i)],
            requests_per_connection,
            std::cref(message),
            std::ref(gate),
            std::ref(
                results[static_cast<size_t>(i)]));
    }

    // 等待所有 worker 准备完成
    {
        std::unique_lock<std::mutex> lock(
            gate.mutex);

        gate.cv.wait(
            lock,
            [&gate, connections]
            {
                return gate.ready == connections;
            });
    }

    // 从这里开始正式计时
    auto benchmark_begin =
        std::chrono::steady_clock::now();

    {
        std::lock_guard<std::mutex> lock(
            gate.mutex);

        gate.start = true;
    }

    gate.cv.notify_all();

    // 等待所有 worker 完成
    for (auto &thread : workers)
        thread.join();

    auto benchmark_end =
        std::chrono::steady_clock::now();

    // 汇总结果
    uint64_t total_completed = 0;
    uint64_t total_errors = 0;

    std::vector<long long> all_latencies;

    size_t total_requests =
        static_cast<size_t>(connections) *
        static_cast<size_t>(
            requests_per_connection);

    all_latencies.reserve(total_requests);

    for (const auto &result : results)
    {
        total_completed += result.completed;
        total_errors += result.errors;

        all_latencies.insert(
            all_latencies.end(),
            result.latency_us.begin(),
            result.latency_us.end());
    }

    double elapsed =
        std::chrono::duration<double>(
            benchmark_end - benchmark_begin)
            .count();

    // QPS
    double qps =
        elapsed > 0.0
            ? static_cast<double>(
                  total_completed) /
                  elapsed
            : 0.0;

    // MB/s
    double throughput_mb =
        elapsed > 0.0
            ? static_cast<double>(
                  total_completed) *
                  static_cast<double>(
                      message_size) /
                  elapsed /
                  1024.0 /
                  1024.0
            : 0.0;

    // P50 / P99
    long long p50 =
        percentile(all_latencies, 0.50);

    long long p99 =
        percentile(all_latencies, 0.99);

    std::cout
        << "\n========== TCP Benchmark ==========\n";

    std::cout
        << "Connections         : "
        << connections
        << "\n";

    std::cout
        << "Requests/connection : "
        << requests_per_connection
        << "\n";

    std::cout
        << "Total requests      : "
        << total_requests
        << "\n";

    std::cout
        << "Message size        : "
        << message_size
        << " bytes\n";

    std::cout
        << "Completed           : "
        << total_completed
        << "\n";

    std::cout
        << "Errors              : "
        << total_errors
        << "\n";

    std::cout
        << "Time                : "
        << elapsed
        << " s\n";

    std::cout
        << "QPS                 : "
        << qps
        << "\n";

    std::cout
        << "Throughput          : "
        << throughput_mb
        << " MB/s\n";

    std::cout
        << "P50                 : "
        << p50 / 1000.0
        << " ms\n";

    std::cout
        << "P99                 : "
        << p99 / 1000.0
        << " ms\n";

    std::cout
        << "===================================\n";

    return total_errors == 0 ? 0 : 2;
}