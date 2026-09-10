#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>

int main()
{
        //1.创建Socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(server_fd==-1)
    {
        perror("socket");
        return 1;
    }

    //2.创建服务器结构体
    sockaddr_in server_addr{};

    //3.设置地址族
    server_addr.sin_family = AF_INET;

    //4.设置IP地址
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //5.设置端口
    server_addr.sin_port = htons(8080);

    //6.绑定Socket和 IP  +  端口
    if(bind(server_fd,(sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    //7.开始监听
    if(listen(server_fd,10)==-1)
    {
        perror("listen");
        close(server_fd);
        return 1;

    }

        //fd集合
    fd_set read_fds;
    fd_set now_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);

    int client_fd = -1;
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int max_fd = server_fd;
    while (true)
    {
        now_fds = read_fds;
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        select(max_fd + 1, &now_fds, nullptr, nullptr,&timeout);

        for (int fd = 0; fd <= max_fd;fd++)
        {
            if(!FD_ISSET(fd,&now_fds))
                continue;

            //判断是否有新客户端连接
            else if (fd==server_fd)
            {
                client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
                if (client_fd == -1)
                {
                    perror("accept");
                    close(server_fd);
                    return 1;
                }
                FD_SET(client_fd, &read_fds);

                if (client_fd > max_fd)
                    max_fd = client_fd;
            }

            //客户端传输数据
            else{

                char buffer[1024]{};

                int n = recv(fd, buffer, sizeof(buffer) - 1, 0);

                if (n == -1)
                {
                    perror("recv");
                    close(fd);
                    
                    return 1;
                }
                else if(n==0)
                {
                    std::cout << "client close" << std::endl;
                    close(fd);
                    FD_CLR(fd, &read_fds);
                    continue;
                }
                // 10.返回数据
                int send_n = send(fd, buffer, n, 0);
                if (send_n == -1)
                {
                    perror("send");
                    close(fd);

                    return 1;
                }
                std::cout << "recv bytes = " << n << std::endl;
                std::cout << "recv data = " << buffer << std::endl;
                std::cout << "send bytes = " << send_n << std::endl;
               
               
            }

        }
            
    }

    close(server_fd);

    return 0;
}