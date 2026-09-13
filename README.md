# Evolutionary C++ Network Library

## 一、学习目标

本项目作为 C++ 网络编程与高性能服务器开发的系统学习项目，通过从简单的 TCP 阻塞式服务器开始，逐步实现一个功能更加完善、性能更加优秀的 C++ 网络库。

通过项目的持续迭代，重点掌握以下内容：

- Linux Socket 网络编程
- TCP 客户端/服务器通信流程
- 阻塞 I/O 与非阻塞 I/O
- I/O 多路复用机制
- select / epoll 的使用与原理
- Reactor 网络模型
- EventLoop 事件循环
- Connection 连接管理
- Buffer 网络缓冲区
- 多线程与并发编程
- 线程池与任务调度
- 定时器
- 内存池与内存管理
- 高性能服务器的性能分析与优化
- io_uring 等新型 I/O 技术

最终目标不是简单完成一个网络库，而是理解：

1. 一个 TCP 服务器是如何工作的；
2. 为什么阻塞 I/O 无法满足高并发需求；
3. select、epoll 等 I/O 多路复用技术解决了什么问题；
4. Reactor 网络模型为什么能够提高服务器的并发处理能力；
5. 网络库中的 Buffer、Connection、EventLoop 等模块为什么存在；
6. 多线程应该如何与 I/O 多路复用结合；
7. 如何通过 Benchmark、QPS、P99 延迟、CPU 和内存等指标分析系统性能；
8. 如何根据实际问题不断优化网络库的架构和性能。

---

## 二、项目演进路线

本项目采用由简单到复杂、由问题驱动技术演进的方式进行学习。

### Stage 1：Blocking I/O Echo Server

实现最基础的单线程 TCP Echo Server。

核心流程：

socket -> bind -> listen -> accept -> recv -> send -> close

主要学习：

- Socket 的基本概念
- TCP 服务端通信流程
- 文件描述符
- socket / bind / listen / accept
- recv / send / close
- 阻塞 I/O
- 客户端与服务端的数据交互

本阶段重点解决：

> 如何从零实现一个能够正常工作的 TCP 服务器？

并通过多个客户端连接实验，观察阻塞 I/O 带来的问题。

---

### Stage 2：引入 Select 并完成网络库初步模块化

## 阶段目标：

将第一阶段的阻塞式 TCP Server 改造成基于 `select` 的 I/O 多路复用模型，并对 Socket、连接管理、I/O 多路复用、事件循环等功能进行模块化封装。

---

## 主要完成内容：

### 1. 引入 select

- 使用 `select()` 监听多个文件描述符
- 实现服务器同时处理多个客户端连接
- 理解 `fd_set`、`FD_SET`、`FD_ISSET`、`FD_ZERO`
- 理解 `select()` 返回值：

  - `>0`：有文件描述符就绪
  - `=0`：超时
  - `<0`：发生错误


---

### 2. Buffer

- 对客户端接收的数据进行缓冲
- 引入 `read_index` 管理已读取数据
- 将网络 I/O 与数据存储进行分离
- 为后续协议解析提供基础


---

### 3. Connection

- 封装单个客户端连接
- 将 `recv()`、`send()` 等操作封装到 `Connection` 类中
- 统一管理客户端 fd 和通信数据

结构：

```
Connection

    |
    |-- fd

    |-- Buffer

    |-- recv_data()

    |-- send_data()
```


---

### 4. ConnectionManager

- 统一管理多个 `Connection`
- 实现客户端连接的创建、查找和删除
- 管理 Connection 生命周期


---

### 5. SocketListener

- 封装：

```
socket → bind → listen
```

- 对外提供监听 Socket
- 降低上层代码对 Socket API 的依赖


---

### 6. SelectPoller

- 封装 `select()` 相关逻辑
- 管理监听的文件描述符集合
- 将 I/O 多路复用逻辑与业务代码分离

为后续替换：

```
select

↓

epoll
```

提供基础。


---

### 7. EventLoop

- 封装网络事件循环
- 负责：

  - 监听新客户端连接
  - 检测客户端数据事件
  - 分发对应事件


开始形成事件驱动网络结构。


---

### 8. IOEvent事件抽象

将原来的简单返回值：

```
成功 / 失败
```

升级为：

```cpp
DATA
CLOSE
ERROR
```

用于区分：

- 收到数据
- 客户端关闭
- 网络错误


---

### 9. Callback网络层与业务层解耦

引入：

```cpp
std::function<void(Connection*)>
```

实现：

```
EventLoop

      ↓

Connection

      ↓

Callback

      ↓

业务处理
```

使网络层负责连接管理，业务层负责具体请求处理。


---

## 第二阶段最终结构：

```
             EventLoop

                 │

      ┌──────────┴──────────┐

      │                     │

SocketListener       SelectPoller

      │                     │

socket/bind/listen      select()

      │                     │

      └──────────┬──────────┘

                 │

        ConnectionManager

                 │

      ┌──────────┼──────────┐

      │          │          │

 Connection Connection Connection

      │          │          │

 Buffer      Buffer      Buffer

```

---

## 阶段核心理解：

第二阶段：

```
SocketListener

        ↓

EventLoop

        ↓

SelectPoller

        ↓

select()监听多个fd

        ↓

ConnectionManager

        ↓

多个Connection

        ↓

Buffer

        ↓

Callback

        ↓

业务处理
```

完成了：

```
简单TCP Server

        ↓

模块化网络Server
```

的转变。

---

## 当前模型存在的问题：

select模型仍存在限制：

- fd数量受 `FD_SETSIZE` 限制
- 每次需要遍历所有fd
- fd集合需要复制，存在额外开销


下一阶段将进行：

```
select

↓

epoll

↓

Reactor模型
```

升级。
### Stage 3：epoll + 多线程（未实现）

针对 select 在大量连接场景下的性能和使用限制，引入 Linux 下更加高效的 epoll。

核心技术：

- epoll
- Reactor
- EventLoop
- 多线程
- 线程池
- 连接管理
- 事件分发

主要目标：

> 构建基于 epoll + 多线程的高并发网络服务器。

---

### Stage 4：性能分析与优化

在网络库基本功能完成后，通过实际测试分析系统瓶颈。

重点关注：

- QPS
- 吞吐量
- P99 延迟
- CPU 使用率
- 内存占用
- 并发连接数

通过 Benchmark 对不同实现进行对比，寻找性能瓶颈，并针对性进行优化。

主要目标：

> 从“能够运行”进一步提升到“能够分析性能并进行优化”。

---

### Stage 5：epoll → io_uring

进一步学习 Linux 新型异步 I/O 技术。

主要学习：

- io_uring
- 异步 I/O
- Submission Queue
- Completion Queue
- 与 epoll 的模型对比

主要目标：

> 理解传统 I/O 多路复用与现代异步 I/O 模型之间的区别。

---

### Stage 6：内存管理与性能优化

针对网络库运行过程中频繁创建和释放对象产生的内存分配开销，引入内存优化。

主要学习：

- 内存池
- 对象池
- 内存分配与释放
- Valgrind
- Cache Line
- False Sharing
- Cache Line 对齐

主要目标：

> 减少动态内存分配开销，并理解 CPU Cache 对高性能程序的影响。

---

### Stage 7：协议层与数据处理优化

进一步完善网络库的数据处理能力。

主要学习：

- 协议解析
- 零拷贝
- Buffer 管理
- 引用计数
- 数据转发

主要目标：

> 降低网络数据处理过程中的拷贝和内存管理开销，提高整体性能。

---

## 三、整体演进过程

```text
Stage 1
Blocking I/O
TCP Echo Server
        ↓
发现问题：
一个客户端可能阻塞整个服务器
        ↓
Stage 2
select
I/O 多路复用
网络库基础抽象
        ↓
发现问题：
select 在大量连接场景下存在性能限制
        ↓
Stage 3
epoll + 多线程
Reactor
高并发网络库
        ↓
Stage 4
Benchmark
性能分析与优化
        ↓
Stage 5
io_uring
现代异步 I/O
        ↓
Stage 6
内存池
Cache Line
False Sharing
内存优化
        ↓
Stage 7
零拷贝
引用计数
协议层优化
        ↓
最终目标
高性能 C++ 网络库
```
