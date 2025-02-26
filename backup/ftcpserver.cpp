#include "stcp.h"

int main(int argc,char * argv[])
{
    if(argc < 2)
    {
        printf("使用 文件 监听端口 的方式使用！\n");
        return -1;
    }
    stcp tcpserver;
    while(1)
    {
        if(!tcpserver.Accept(atoi(argv[1])))
            return -1;
        file filebuf;
        memset(&filebuf,0,sizeof(file));
        if(tcpserver.Recv(&filebuf,sizeof(file)) == 0)
            return -1;
        std::cout<<filebuf.name<<":"<<filebuf.fileinfo.st_size<<"\n";
        if(tcpserver.Send("ok") == 0)
            return -1;
        if(tcpserver.Recvfile(filebuf) == 0)
            return -1;
        printf("Recv File Done!\n");
        if(tcpserver.Send("ok") == 0)
            return -1;
        printf("Recv file done!\n");
    }
    return 0;
}