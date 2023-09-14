#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>


// #using namespace std;


int main() {

    // 创建套接字，指定IPv4协议族，指定流格式套接字类型，指定传输层协议TCP
    int listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_addr; 

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(server_addr));
    server_addr.sin_port = htons(2345);

    /*  setsockopt是一个用于设置套接字选项的函数，用于更改底层套接字的特定行为。
    /   - sockfd：需要设置选项的套接字描述符。
    /   - level：选项所在的协议层。常用的如：SOL_SOCKET，IPPROTO_IP，IPPROTO_TCP等。
    /   - optname：需要访问的选项名。
    /   - optval：对于optname选项设置的值的指针。
    /   - optlen：现在optval指向的值的大小。
    /   返回值：如果成功，返回0；如果出错，返回-1。
    /   SO_REUSEADDR 使套接字能和已使用的地址绑定，适合服务端重启后重新绑定
    */   
    int enable_so_reuseaddr = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, 
                &enable_so_reuseaddr, sizeof(enable_so_reuseaddr));

    // 将套接字与特定的IP地址和端口号绑定                
    bind(listenSock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 


}