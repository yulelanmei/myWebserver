#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>   //网络编程，客户端信息转换用
#include<unistd.h>      //为类Unix系统中的C/C++提供系统调用api

int main() {

    // 创建套接字，指定IPv4协议族，指定流格式套接字类型，指定传输层协议（0为选择套接字类型对应的默认协议）
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 套接字地址描述结构，用于初始化向服务端发送请求
    struct sockaddr_in server_addr;

    


}