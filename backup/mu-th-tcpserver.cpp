#include "stcp.h"
#include "xthread_pool.h"

class Ser:public XTask<void>
{
public:

    stcp tcpserver;
    int Run() override
    {
        std::osyncstream sy_out(std::cout);
        std::string buff;
        if(!tcpserver.Recv(buff))
            return -1;
        sy_out<<buff.data()<<"\n";

        if(!tcpserver.Send("Hello"))
            return -1;
        
        sy_out<<"Close------------------------------------------\n";
        tcpserver.CloseCfd();
        return 0;
    }

};
int main(int argc,char * argv[])
{
    if(argc < 2)
    {
        printf("使用 文件 监听端口 的方式使用！\n");
        return -1;
    }
    std::shared_ptr<Ser> server = std::make_shared<Ser>();
    if(server->tcpserver.Init(atoi(argv[1])) == 0)
        return -1;
    
    //timeval stop = {3,0};
    XThread_Pool<void> pool(12);
    pool.Start();
    //printf("Start!\n");
    std::osyncstream sy_out(std::cout);
    //for(int i = 0;i<10;++i)
    while(1)
    {
        //printf("AddTask\n");
        if(!server->tcpserver.Accept())
        {
            sy_out<<"Not Accept!\n";
            continue;
        }
        else sy_out<<"Accept done!\n";
        pool.AddTask(server);
        //printf("EndAdd\n");
    }
    printf("Die Here!\n");
    pool.Stop();
    printf("Pool Stopped!\n");
    return 0;
}