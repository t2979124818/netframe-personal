#include "ctcp.h"
//ftcp client

int main(int argc,char *argv[])
{
    if(argc < 4)
    {
        printf("请依照 文件 IP 端口 要传输的文件名 的方式运行！\n");
        exit(-1);
    }
    std::string ip = argv[1];
    u_int16_t port = atoi(argv[2]);
    ctcp tcpclient;
    if( 0 == tcpclient.Connect(ip.c_str(),port))
        return -1;

    file filebuf;
    memcpy(filebuf.name,argv[3],sizeof(filebuf.name));
    stat(argv[3],&filebuf.fileinfo);//获得文件属性
    std::cout<<filebuf.name<<":"<<filebuf.fileinfo.st_size<<":"<<sizeof(filebuf.name)<<"\n";

    tcpclient.Send(&filebuf,sizeof(filebuf));
    
    std::string conf;
    if(tcpclient.Recv(conf) == 0)
        return -1;
    if(conf != "ok")
    {
        printf("1st Check Error!\n");
        return -1;
    }
    conf.clear();
    if(tcpclient.Sendfile(filebuf)==0)
        return -1;
    if(tcpclient.Recv(conf) == 0)
        return -1;
    if(conf != "ok")
    {
        printf("Double Check Error!\n");
        return -1;
    }
    printf("Send File done!\n");
    return 0;
}