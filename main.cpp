#include "requestData.h"
#include "epoll.h"
#include "threadpool.h"
#include "util.h"
#include "config.h"
#include <sys/epoll.h>
#include <queue>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <memory>



using namespace std;

static const int MAXEVENTS = 5000;
static const int LISTENQ = 1024;
const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;

const int PORT = 8888;
const int ASK_STATIC_FILE = 1;
const int ASK_IMAGE_STITCH = 2;

const int TIMER_TIME_OUT = 500;

void acceptConnection(int listen_fd, int epoll_fd, const string &path);

int socket_bind_listen(int port)
{
    // 检查port值，取正确区间范围
    if (port < 1024 || port > 65535)
        return -1;

    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    // 消除bind时"Address already in use"错误
    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;

    // 设置服务器IP和Port，和监听描述副绑定
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    // 开始监听，最大等待队列长为LISTENQ
    if(listen(listen_fd, LISTENQ) == -1)
        return -1;

    // 无效监听描述符
    if(listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}


int main()
{
#ifndef _PTHREADS // 如果没有定义 _PTHREADS 宏
    printf("_PTHREADS is not defined !\n"); // 输出警告信息
#endif

handle_for_sigpipe(); // 处理 SIGPIPE 信号

if (Epoll::epoll_init(MAXEVENTS, LISTENQ) < 0) // 初始化 epoll，设置监听队列的长度和最大事件数
{
    perror("epoll init failed");
    return 1;
}

if (ThreadPool::threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE) < 0) // 创建线程池，设置线程数和队列长度
{
    printf("Threadpool create failed\n");
    return 1;
}

int listen_fd = socket_bind_listen(PORT); // 创建套接字并绑定到端口
if (listen_fd < 0) // 如果绑定失败
{
    perror("socket bind failed");
    return 1;
}

if (setSocketNonBlocking(listen_fd) < 0) // 将套接字设置为非阻塞模式
{
    perror("set socket non block failed");
    return 1;
}

shared_ptr<RequestData> request(new RequestData()); // 创建一个 RequestData 对象，用于存储连接信息
request->setFd(listen_fd); // 将套接字文件描述符存储到 RequestData 对象中

if (Epoll::epoll_add(listen_fd, request, EPOLLIN | EPOLLET) < 0) // 将套接字添加到 epoll 中进行监听
{
    perror("epoll add error");
    return 1;
}

while (true) // 进入无限循环，监听传入连接
{
    Epoll::my_epoll_wait(listen_fd, MAXEVENTS, -1); // 等待事件发生，阻塞当前线程

    // 事件发生后，将连接信息添加到线程池进行处理

    //ThreadPool::threadpool_destroy(); // 可以在需要时销毁线程池
    //break; // 在需要时跳出循环
}

return 0; // 程序正常结束，返回 0

}
