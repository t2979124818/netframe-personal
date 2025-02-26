#include "ctcp.h"

int main(int argc,char * argv[])
{
    if(argc < 3)
    {
        printf("请依照 文件 IP 端口 的方式运行！\n");
        exit(-1);
    }
    std::string ip = argv[1];
    u_int16_t port = atoi(argv[2]);
    ctcp tcpclient;
    if( 0 == tcpclient.Connect(ip.c_str(),port))
        return -1;

    if(tcpclient.Send("HHHHHHHHHHHHHHHHHHHHHHHH") == 0)
        return -1;
    
    std::string buff;
    if(!tcpclient.Recv(buff))
        return -1;
    std::cout<<buff<<'\n';
    

    return 0;
}