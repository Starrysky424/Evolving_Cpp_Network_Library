2026年9月10日
今日进度：

1. 添加 Buffer.h / Buffer.cpp
   - 对客户端读写的数据进行缓冲区封装。
   - 将网络 I/O 与数据处理解耦，避免直接操作 recv/send。

2. 添加 Connection.h / Connection.cpp
   - 将每一个客户端连接封装成一个 Connection 对象。
   - 一个 fd 对应一个 Connection。
   - 封装客户端的 recv/send 操作。

3. 添加 ConnectionManager.h / ConnectionManager.cpp
   - 统一管理所有客户端连接。
   - 根据 fd 查找、添加、删除对应的 Connection。
   - 实现 fd → Connection 的映射。

4. 与 select 服务器结合
   - select 负责监听哪些 fd 有事件。
   - ConnectionManager 负责找到对应的 Connection。
   - Connection 负责具体的客户端数据收发。
   - 初步实现网络事件监听、连接管理、数据收发的分层。

今日理解：
select 负责“发现哪个客户端有事件”，ConnectionManager 负责“找到哪个客户端对象”，Connection 负责“处理这个客户端的数据”。



2026年9月12日
今日进度
1. 封装 SocketListener
   - 添加 SocketListener.h / SocketListener.cpp
   - 封装 socket → bind → listen 服务器初始化流程。
2. 封装 SelectPoller
   - 添加 SelectPoller.h / SelectPoller.cpp
   - 封装 select() 多路 I/O 复用机制。
   - 管理监听 fd、就绪 fd 以及 select() 超时。
3. 引入 EventLoop
   - 添加 EventLoop.h / EventLoop.cpp
   - 封装服务器事件循环。
   - 将新客户端连接处理和客户端数据处理从 main() 中拆分出来。
4. 实现模块解耦
   - SocketListener：负责服务器 Socket 初始化
   - SelectPoller：负责 I/O 多路复用
   - ConnectionManager：负责连接管理
   - Connection：负责客户端数据收发
   - EventLoop：负责事件循环与事件分发

今日收获

将原本集中在 main() 中的网络代码进行模块化拆分，使各模块职责更加清晰，为后续替换 select、引入其他 I/O 多路复用模型以及进一步完善网络库架构打下基础。