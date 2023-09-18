#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<fcntl.h>

#define MAX_IP_ADDR_LEN 16

void set_epoll_events(int epoll_fd, int fd, int event, int op) {
    /*
    epoll_event是一个结构体，用于表示epoll中的事件。其成员变量有evens和data
    events字段是一个位掩码，表示了要监听的事件类型：
    - EPOLLIN：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
    - EPOLLOUT：表示对应的文件描述符可以写；
    - EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
    - EPOLLERR：表示对应的文件描述符发生错误；
    - EPOLLHUP：表示对应的文件描述符被挂断；
    - EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的；
    - EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里。
    data字段是一个联合体，用于存储用户数据。这个数据在调用epoll_ctl时设置，然后在epoll_wait返回时可以获取。
    这个字段通常用于存储与事件相关的文件描述符。
    */
    epoll_event epoll_ev;
    epoll_ev.data.fd = fd;
    epoll_ev.events = event | EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLONESHOT;

    /*
    epoll_ctl函数用于控制epoll的行为，包括添加、修改和删除事件。
    参数：
    - epfd：由epoll_create函数返回的epoll文件描述符。
    - op：操作类型，可以是以下几种：
    - EPOLL_CTL_ADD：注册新的fd到epfd中。
    - EPOLL_CTL_MOD：修改已经注册的fd的监听事件。
    - EPOLL_CTL_DEL：从epfd中删除一个fd。
    - fd：需要操作的文件描述符。
    - event：指向epoll_event结构体的指针，指定监听类型。
    */
    if (epoll_ctl(epoll_fd, op, fd, &epoll_ev) < 0) {
        perror("Operate epoll_fd Error!\n");
    }
}

//  将文件描述符添加进入epoll中
void Add_FD_in_epoll(int epoll_fd, int fd, int event) {
    set_epoll_events(epoll_fd, fd, event, EPOLL_CTL_ADD);
    set_fd_status(fd, O_NONBLOCK);  //  对文件描述符设置为非阻塞
}

// 用于设置套接字非阻塞
int set_fd_status(int fd, int status) {
    /*
    fcntl 是 Unix-like 系统中的一个函数，全名为 "file control"，用于对打开的文件描述符进行各种操作。
    fcntl 可以执行的操作包括：
    - F_DUPFD：复制文件描述符。
    - F_GETFD：获取文件描述符的标志。
    - F_SETFD：设置文件描述符的标志。
    - F_GETFL：获取文件状态标志。
    - F_SETFL：设置文件状态标志。
    - F_GETOWN：获取接收 SIGIO 和 SIGURG 信号的进程 ID 或进程组 ID。
    - F_SETOWN：设置接收 SIGIO 和 SIGURG 信号的进程 ID 或进程组 ID。

    其他的文件状态标志:
    - O_NONBLOCK：所有对该文件的操作都不会阻塞。
    - O_RDONLY：以只读方式打开文件。
    - O_WRONLY：以只写方式打开文件。
    - O_RDWR：以读写方式打开文件。
    - O_APPEND：在写入时将数据追加到文件的末尾。
    - O_CREAT：如果文件不存在，则创建文件。
    - O_TRUNC：如果文件已存在，并且是以写入或者读写方式打开的，则将其长度截断为0。
    - O_EXCL：与 O_CREAT 一起使用，如果文件已存在，则打开失败。
    - O_SYNC：以同步的方式打开文件，任何写入操作都将等待物理 I/O 操作完成。
    - O_DSYNC：同步写入，只有在写入操作涉及文件的数据部分时，才等待物理 I/O 操作完成。
    - O_RSYNC：同步读写，任何读取操作都将等待与文件相关的写入操作完成。
    */
   int old_fd_status = fcntl(fd, F_GETFL); // 获取fd的文件状态标志
   int new_fd_status = old_fd_status | status;  
   fcntl(fd, F_SETFL, new_fd_status);
   return old_fd_status;
}

int main() {

    // 创建套接字，指定IPv4协议族，指定流格式套接字类型，指定传输层协议TCP
    int listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock < 0) {
        perror("Server Create ListenSock Error!\n");
    }

    struct sockaddr_in server_addr; 

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    char Client_IP_addr[MAX_IP_ADDR_LEN] = "127.0.0.1";
    inet_pton(AF_INET, Client_IP_addr, &(server_addr));
    server_addr.sin_port = htons(2345);

    /*
    /   setsockopt是一个用于设置套接字选项的函数，用于更改底层套接字的特定行为。
    /   - sockfd：需要设置选项的套接字描述符。
    /   - level：选项所在的协议层。常用的如：SOL_SOCKET，IPPROTO_IP，IPPROTO_TCP等。
    /   - optname：需要访问的选项名。
    /   - optval：对于optname选项设置的值的指针。
    /   - optlen：现在optval指向的值的大小。
    /   返回值：如果成功，返回0；如果出错，返回-1。
    /   SO_REUSEADDR 使套接字能和已使用的地址绑定，适合服务端重启后重新绑定，后续支持实现复用
    */   
    int enable_so_reuseaddr = 1;
    setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, 
                &enable_so_reuseaddr, sizeof(enable_so_reuseaddr));

    // 将套接字与特定的IP地址和端口号绑定                
    if (bind(listenSock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Server Bind Error!\n");
    }

    
    // 使一个无连接的套接字进入监听状态；设置未完成连接队列最大长度
    int max_unconnected_queue_length = 20;
    if (listen(listenSock, max_unconnected_queue_length) < 0) {
        perror("Server Listen Error!\n");
    }

    // 创建一个epoll实例，参数size在Linux2.6.8已无意义，原本用于设置监控的文件描述符数量
    int epoll_fd = epoll_create(0);
    if (epoll_fd < 0) {
        perror("Create Epoll FD Error!\n");
    }
    
    

    /*
    /   accept函数从已完成连接队列队头取出一个socket。
    /   如果已完成连接队列为空，那么阻塞等待。
    /   - sockfd：套接字的文件描述符，这个套接字在listen后处于监听状态。
    /   - addr：（可选）指向一个结构的指针，该结构将被填充与已接受连接的对端的地址信息。
    /   - addrlen：（可选）是一个值-结果参数，初始时为addr所指向的缓冲区的大小，返回时为存储在该缓冲区的地址结构的实际大小（以字节为单位）。
    /   - 如果函数调用成功，返回一个新的套接字文件描述符。这个新的描述符代表与客户端的新的连接。这个连接是独立于监听套接字的。
    /   - 如果函数调用失败，返回-1。                    
    */
    struct sockaddr_in cilent_addr;
    socklen_t cilent_addr_size = sizeof(cilent_addr);
    int clientSock = accept(listenSock, 
                    (struct sockaddr*)&cilent_addr, &cilent_addr_size);
    if (clientSock < 0) {
        perror("Server Accept Error!\n");
    }                    

    /*
    write函数用于向一个已连接的套接字写入数据。
    - fd：已连接的套接字的文件描述符。
    - buf：指向要发送数据的缓冲区的指针。
    - count：要写入的字节数。
    - 如果函数调用成功，返回实际写入的字节数。这可能少于请求的字节数，因为可能会发生信号中断或其他因素。
    - 如果函数调用失败，返回-1。
    */
    char* massage_to_be_send = "Hello Clinet";
    ssize_t Bytes_Written_in = write(clientSock, massage_to_be_send, sizeof(massage_to_be_send));
    if (Bytes_Written_in < 0) {
        perror("Server Write Error!\n");
    } else {
        printf("Server Write %d in Socket\n");
    }

    // 关闭两个套接字
    if (close(listenSock) < 0) {
        perror("Server Close listenSock Error!\n");
    }
    if (close(clientSock) < 0) {
        perror("Serve Close clientSock Error!\n");
    }

    return 0;

}