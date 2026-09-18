#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string>
#include"Buffer.h"
#include"Decoder.h"
void send_message(int fd,const std::string &message)
{
    uint32_t length = message.size();
    length = htonl(length);

    // 传输协议头，还未发送完整数据
    if (send(fd, &length, sizeof(length), 0) == -1)
    {
        perror("send length");
        close(fd);
        return ;
    }

    // 传输完整消息
    if (send(fd, message.data(), message.size(), 0) == -1)
    {
        perror("send message");
        close(fd);
        return ;
    }
}
int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd==-1)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if(connect(fd,(sockaddr *)&server_addr,sizeof(server_addr))==-1)
    {
        perror(("connect"));
        close(fd);
        return 1;
    }

    std::string message1(5000,'A');
    std::string messag2 = "WORLD";
    send_message(fd, message1);
    send_message(fd, messag2);

    Buffer input_buffer;
    Decoder decoder;

    int message_count = 0;

    while (message_count<2)
    {
        char buffer[1024]{};
        int n = recv(fd, buffer, sizeof(buffer) - 1, 0);

        if (n > 0)
        {
            input_buffer.add_data(buffer, n);
            std::string message;

            while(decoder.decode(input_buffer,message))
            {
                std::cout << "server reply:" << message << std::endl;
                message.clear();
                message_count++;
            }
                }

        else if (n == 0)
        {
            std::cout << "server closed connection" << std::endl;
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }
    
    std::cout << "connected to server" << std::endl;

    close(fd);
    return 0;
}