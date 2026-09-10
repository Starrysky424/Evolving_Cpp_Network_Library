#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>

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
    server_addr.sin_port = htons(8888);

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
    
    //8.接收客户端连接
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);

    if(client_fd==-1)
    {
        perror("accept");
        close(server_fd);
        return 1;
    }

    //9.接收数据
    char buffer[1024]{};

    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if(n==-1)
    {
        perror("recv");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    //10.返回数据
    int send_n = send(client_fd, buffer, n, 0);
    if(send_n ==-1 )
    {
        perror("send");
        close(client_fd);
        close(server_fd);
        return 1;
    }
    std::cout << "recv bytes = " << n << std::endl;
    std::cout << "recv data = " << buffer << std::endl;
    std::cout << "send bytes = " << send_n << std::endl;
    close(client_fd);
    close(server_fd);
    return 0;
}