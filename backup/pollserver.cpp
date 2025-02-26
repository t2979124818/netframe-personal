#include "stcp.h"
#include <poll.h>

#define MAX_FDS 1024
#define MAX_BUF 1024

int main(int argc,char *argv[])
{
    if(argc < 2)
    {
        printf("以 文件 监听端口 的方式使用！\n");
        return -1;
    }
    stcp tcpserver;
    int listen = tcpserver.Init(atoi(argv[1]));
    if(listen < 0)
        return -1;
    
    pollfd fds[MAX_FDS];
    for(int i = 0;i<MAX_FDS;++i)
        fds[i].fd = -1;

    fds[listen].fd = listen;
    fds[listen].events = POLLIN; //POLLIN读，POLLOUT写,POLLIN|POLLOUT读写

    int maxfd = listen;

    while(1)
    {
        //相比select少了一次备份的拷贝，但仍低效，且监视方法仍然与select相同，是遍历
        int eventofds = poll(fds,maxfd + 1,-1);//-1不设置超时，单位是ms
        if(eventofds < 0)
        {
            printf("pool fail!\n");
            break;
        }
        if(0 == eventofds)
        {
            printf("poll timeout!\n");
            continue;
        }
        for(int eventfd = 0;eventfd<maxfd;++eventfd)
        {
            if(fds[eventfd].fd < 0)
                continue;
            if(fds[eventfd].revents & POLLIN == 0)
                continue;
            
            if(eventfd == listen)
            {
                sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int client = accept(listen,(sockaddr *)&clientaddr,&len);
                if(client < 0)
                {
                    printf("accept error!\n");
                    continue;
                }

                fds[client].fd = client;
                fds[client].events = POLLIN;
                if(maxfd < client)
                    maxfd = client;
            }
            if(eventfd != listen)
            {
                char buff[MAX_BUF];
                memset(buff,0,sizeof(buff));
                if(recv(eventfd,buff,MAX_BUF,0) <= 0)
                {
                    close(eventfd);
                    fds[eventfd].fd = -1;
                    if(maxfd == eventfd)
                    {
                        for(int j = maxfd;j>0;--j)
                        {
                            if(fds[j].fd != -1)
                            {
                                maxfd = j;
                                break;
                            }
                        }
                    }
                    printf("Disconnect!\n");
                }else
                {
                    printf("client%d:%s",eventfd,buff);
                    send(eventfd,buff,sizeof(buff),0);
                }
            }
        }
    }

    return 0;
}