#include "xthread_pool.h"

class Task:public XTask<void>
{
    int Run() override
    {
        std::osyncstream sy_out(std::cout);
        sy_out<<"thread:"<<std::this_thread::get_id()<<"\n";
        return 1;
    }
};

int main()
{
    Task t;
    XThread_Pool<void > pool(12);
    pool.Start();
    for(int i = 0;i<10;++i)
        pool.AddTask(std::make_shared<Task>(t) );
    pool.Stop();
    return 0;
}