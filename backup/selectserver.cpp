#include "stcp.h"

int main(int argc,char * argv[])
{
    if(argc < 2)
    {
        printf("使用 文件 监听端口 的方式使用！\n");
        return -1;
    }
    stcp server;
    int linstensock = server.Init(atoi(argv[1]));
    if(linstensock <= 0)
        return -1;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(linstensock,&readfds);

    int maxfd = linstensock;
    while(1)
    {
        timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        fd_set tmpfds = readfds;//每次都要备份，拷贝低效
        //int evento = select(maxfd + 1,&tmpfds,NULL,NULL,&timeout);
        int evento = select(maxfd + 1,&tmpfds,NULL,NULL,0);//select运行在内核态，每次运行都要将socket集从用户态拷贝到内核态，低效！
        if(evento < 0)
            printf("Select fail!\n");
        if(evento == 0)
            printf("select timeout!\n");
        for(int eventfd = 0;eventfd <= maxfd;++eventfd)
        {
            if(FD_ISSET(eventfd,&tmpfds) == 0)
                continue;
            if(eventfd == linstensock)
            {
                sockaddr_in clientaddr;
                int client;
                socklen_t len = sizeof(sockaddr);
                if(-1 == (client = accept(linstensock,(sockaddr *)&clientaddr,&len)))
                {
                    printf("Accept Error!\n");
                    continue;
                }
                if(0 == client)
                    continue;
                FD_SET(client,&tmpfds);
                if(maxfd < client)
                    maxfd = client;
            }
            if(eventfd != linstensock)// 要么是0,要么是服务端的socket,要么是客户端连接的socket
            {
                char buff[1024];
                memset(buff,0,sizeof(buff));
                if(0 >= recv(eventfd,buff,sizeof(buff),0))
                {
                    close(eventfd);
                    FD_CLR(eventfd,&tmpfds);
                    if(eventfd == maxfd)
                    {
                        for(int j = maxfd;j>0;--j)
                        {
                            if(FD_ISSET(j,&tmpfds)==1)
                            {
                                maxfd = j;
                                break;
                            }
                        }
                    }
                    printf("disconnect!\n");
                    continue;
                }else
                {
                    printf("fd:%d:%s\n",eventfd,buff);
                    send(eventfd,buff,strlen(buff),0);
                }

                

            }

        }
    }
    return 0;
}