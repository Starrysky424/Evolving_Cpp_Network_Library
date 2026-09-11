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
