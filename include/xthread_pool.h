#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <list>
#include <functional>
#include <future>
#include <iostream>
#include <syncstream>

template <typename _T>
class XTask //模板类型为异步参数类型,不使用异步为void
{
    public:
        //任务执行函数
        virtual _T Run() = 0;
        //是否可以退出的函数
        std::function<bool()> is_exit;

        //异步获取值
        _T GetValue(){return _p.get_future().get();}
        //异步传入值
        void SetValue(_T val){_p.set_value(val);}
    private:
        std::promise<_T> _p;
};
template <>
class XTask<void>
{
    public:
        virtual int Run() = 0;
        std::function<bool()> is_exit;

        //异步获取值
        //void GetValue(){return _p.get_future().get();}
        //void SetValue(){_p.set_value(0);}
    private:
        //std::promise<int> _p;
};

template <typename _T>
class XThread_Pool //模板类型为异步参数类型，不使用异步为void
{
public:
    //构造函数，传入参数是线程池最大线程数量
    XThread_Pool(int n):_thread_num(n){}
    //析构函数
    ~XThread_Pool()
    {
        _thread_num = 0;
        //for(auto it:_ths)
        //    delete it;
        _ths.clear();
        //for(auto it:_tasks)
        //    delete it;
        _tasks.clear();

        if(_mgr.joinable())
            _mgr.join();
    }
    //开始制作Run线程并将线程池中的线程实例化
    void Start();
    //管理线程运行停止线程池处理任务
    void Stop();
    //管理线程运行线程池一直空闲多长时间后停止
    void StopAt(timeval &_time);
    //向任务队列加入任务
    void AddTask(std::shared_ptr<XTask<_T>> task);
    void AddTask(XTask<_T> task);
    //返回线程池最大线程数量
    int thread_number(){return _thread_num;}
    //返回线程是否已经退出
    bool is_exit(){return _is_exit;}
    //任务队列中的任务数量
    int wait_task(){return _tasks.size();}
    //执行中的任务数量
    int run_num(){return _run_num;}
    //是否将要退出
    bool Stopped(){return _stop;}
protected:
    //线程池从任务队列中获取任务
    std::shared_ptr<XTask<_T>> GetTask();
private:
    //停止线程池处理任务
    void _Stop();
    //线程池一直空闲多长时间后停止
    void _StopAt(timeval &_time);
    //线程池任务，用于执行XTask类的行为
    void Run();
    //线程池线程数量
    int _thread_num = 0;
    //锁
    std::mutex _mtx;
    //std::vector<std::thread *> _ths;
    //线程池，智能指针更好管理
    std::vector<std::shared_ptr<std::thread>> _ths;
    //条件变量用来保证同步与异步
    std::condition_variable _cv;

    //std::condition_variable _stop;
    //std::mutex _m_stop;
    //std::list<XTask *> _tasks;
    //任务队列
    std::list<std::shared_ptr<XTask<_T>>> _tasks;
    //线程是否已经退出
    bool _is_exit = false;
    //运行中数量,线程安全
    std::atomic<int> _run_num{0};
    //管理线程，用来管理pool,启动与停止线程池都会作为任务被此线程执行
    std::thread _mgr;
    //计时器，如果一直没有任务到时间结束则释放线程池
    timeval _timer;
    //是否将要停止线程池
    std::atomic<bool> _stop{false};
    //Run lock
    //std::mutex _rlm;
};