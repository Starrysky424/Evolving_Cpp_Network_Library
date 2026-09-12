#include<iostream>
#include<sys/socket.h>
#include <unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include"ConnectionManager.h"
#include"SocketListener.h"
#include"SelectPoller.h"
int main()
{
    //监听服务器
    SocketListener socketListener("127.0.0.1", 8080);

    int server_fd = socketListener.fd();

    //创建select分配器
    SelectPoller selectPoller;
    selectPoller.add_fd(server_fd);
    
    ConnectionManager connect_manager;
    while (true)
    {
        int n = selectPoller.poll(5);
        if (n == -1)
        {
            perror("select");
            continue;
            
        }
        else if(n==0)
        {
            std::cout << "select timeout" << std::endl;
            continue;
        }

        for (int fd:selectPoller.get_ready_fds())
        {

          if (fd==server_fd)
          {
              sockaddr_in client_addr{};
              socklen_t client_len = sizeof(client_addr);

              int client_fd = accept(
                  server_fd, (sockaddr *)&client_addr, &client_len);

                if(client_fd==-1)
                {
                    perror("accept");
                    continue;
                    
                }

                selectPoller.add_fd(client_fd);
                connect_manager.add_connection(client_fd);

                std::cout << "new client:" << client_fd << std::endl;
          }

              // 客户端传输数据
              else
              {

                  Connection *client = connect_manager.get_connection(fd);

                  if (client == nullptr)
                      continue;

                  bool check = client->recv_data();
                  if (!check)
                  {
                      connect_manager.delete_connection(fd);
                      selectPoller.remove_fd(fd);
                      close(fd);
                      continue;
                  }

                  std::cout << "success recv" << std::endl;
                  client->send_data();
              }
        }
            
    }

    close(server_fd);

    return 0;
}