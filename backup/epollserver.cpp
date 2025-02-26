#include "stcp.h"
#include <sys/epoll.h>

#define MAX_EVE 1024
#define MAX_BUF 1024

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("以 文件 监听端口 的方式使用！\n");
        return -1;
    }
    stcp tcpserver;
    int listen = tcpserver.Init(atoi(argv[1]));
    if (listen < 0)
        return -1;

    // 边缘触发要使用非阻塞，否则可能阻塞在accept
    // 代码逻辑与poll、select相同
    int epollfd = epoll_create(1);
    epoll_event ev;
    ev.data.fd = listen;
    // ev.data.ptr = (void*)"HELOO";指定数据会与事件一起返回
    // ev.events = EPOLLIN;
    ev.events = EPOLLIN | EPOLLET; // 服务端监听水平触发便可

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listen, &ev);

    epoll_event eps[MAX_EVE];

    while (1) // 事件循环,IO复用中不能被阻塞，否则在阻塞时，不能监听某些事件
    {
        int eveofds = epoll_wait(epollfd, &ev, MAX_EVE, -1); // 与poll类似，-1为不启用超时

        if (eveofds < 0)
        {
            printf("wait fail!\n");
            break;
        }
        if (0 == eveofds)
        {
            printf("wait timeout\n");
            continue;
        }
        for (int evefd = 0; evefd < eveofds; ++evefd)
        {

            if (eps[evefd].events & EPOLLHUP) // 客户端连接关闭
                close(eps[evefd].data.fd);
            else if (eps[evefd].events & (EPOLLIN | EPOLLPRI))
            {
                if (eps[evefd].data.fd == listen)
                {
                    while (1) // 边缘触发可能出现多个连接同时请求的情况而不能处理的情况，所以要循环处理
                    {
                        sockaddr_in clientaddr;
                        socklen_t len = sizeof(clientaddr);
                        int client = accept4(listen, (sockaddr *)&clientaddr, &len, SOCK_NONBLOCK);
                        if (client < 0 && errno == EAGAIN)
                            break;

                        ev.data.fd = client;
                        ev.events = EPOLLIN | EPOLLET; // EPOLLET为边缘触发，默认水平触发
                        epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &ev);
                    }
                }
                if (eps[evefd].data.fd != listen)
                {
                    char buff[MAX_BUF];
                    int readed;
                    int total;
                    tcpserver.SetClient(eps[evefd].data.fd);
                    while (1)
                    {
                        if ((readed = recv(eps[evefd].data.fd, buff + total, sizeof(buff) - total, 0)) <= 0)
                        {
                            if (readed == -1 && errno == EINTR) // 读取时信号中断
                                continue;
                            if (readed < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) // 读完了
                            {
                                tcpserver.Send(buff, MAX_BUF);
                                memset(buff, 0, sizeof(buff));
                            }
                            else
                            {
                                printf("Disconnected!\n");
                                close(eps[evefd].data.fd);
                            }
                            break;

                            // close后epoll会自动把相应的socket删除，不必手动
                            // epoll_ctl(epollfd,EPOLL_CTL_DEL,eps[evefd].data.fd,0);
                        }
                        else
                            total += readed;
                        if (total == MAX_BUF)
                        {
                            tcpserver.Send(buff, MAX_BUF);
                            memset(buff, 0, sizeof(buff));
                        } // 防止一次送不完
                    }
                }
            }
            else if (eps[evefd].events & EPOLLOUT)
            {
                // 先留空吧
            }
            else
                close(eps[evefd].data.fd);
        }
    }

    return 0;
}