#include "stcp.h"

int main(int argc,char * argv[])
{
    if(argc < 2)
    {
        printf("使用 文件 监听端口 的方式使用！\n");
        return -1;
    }
    stcp tcpserver;
    if(tcpserver.Init(atoi(argv[1])) == 0)
        return -1;
    printf("Init!\n");
    while(1)
    {
        if(!tcpserver.Accept())
            return -1;
        int pid = fork();
        if(pid < 0)
        {
            printf("Src Run Out!\n");
            return -1;
        }   
        if(pid == 0)
            continue;
        std::string buff;
        if(!tcpserver.Recv(buff))
            return -1;
        std::cout<<buff.data()<<"\n";

        if(!tcpserver.Send("Hello"))
            return -1;
    }

    return 0;
}