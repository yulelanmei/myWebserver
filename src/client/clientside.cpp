#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>   //网络编程，客户端信息转换用
#include<unistd.h>      //为类Unix系统中的C/C++提供操作系统系统调用api

int main() {

    // 创建套接字，指定IPv4协议族，指定流格式套接字类型，指定传输层协议（0为选择套接字类型对应的默认协议）
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 用于处理网络通信中地址的结构
    struct sockaddr_in server_addr;

    // 初始化内存地址区域的值为0
    bzero(&server_addr, sizeof(server_addr));

    // 协议族：IPv4
    server_addr.sin_family = AF_INET;

    // inet_addr()函数用于将点分十进制的IP地址转换为用于网络传输的长整型数值。但已被inet_pton替代，因为它无法处理IPv6地址。
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.1")
    // inet_pton()函数用于将点分十进制IP地址转换为网络字节序的二进制形式，它可以处理IPv4和IPv6地址。这里设置服务端地址。
    inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));

    // htons()函数用于将主机字节序（Host Byte Order）转换为网络字节序（Network Byte Order）
    // 大多数网络协议使用网络字节序（大端字节序），而主机字节序则取决于机器硬件，因此需要统一。
    // 这里传入端口号
    server_addr.sin_port = htons(2345);

    // connect()函数用于建立与指定socket的连接。
    // 如果连接成功，connect()函数会返回0。如果连接失败，函数会返回-1，并设置errno变量。
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 读取传输过来的数据，read()函数用于从文件描述符中读取数据。
    char RecvBuffer[40];
    read(sock, RecvBuffer, sizeof(RecvBuffer) - 1);

    // 打印信息
    printf("Massage Receive From Server: %s\n", RecvBuffer);

    // 关闭套接字
    close(sock);

    return 0;
}