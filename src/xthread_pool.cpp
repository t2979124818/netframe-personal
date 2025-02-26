
#include "xthread_pool.h"

template <typename _T>
void XThread_Pool<_T>::Run()
{
    //std::osyncstream sy_out(std::cout);
    //sy_out<<"Begin Thread:"<<std::this_thread::get_id()<<std::endl;
    while(!is_exit())
    {
        //std::unique_lock<std::mutex> ul(_rlm);
        std::shared_ptr<XTask<_T>> task = XThread_Pool<_T>::GetTask();
        if(task == nullptr) continue;
        ++_run_num;
        try
        {
            auto re = task->Run();
            task->SetValue(re);
        
        }catch(...)
        {
            std::cerr<<"XTask Run Error\n";
        }
        --_run_num;
        //if(_run_num == 0)
        //    _stop.notify_all();
    }
    //sy_out<<"End Thread:"<<std::this_thread::get_id()<<std::endl;
}

template <>
void XThread_Pool<void>::Run()
{
    //std::osyncstream sy_out(std::cout);
    //sy_out<<"Begin Thread:"<<std::this_thread::get_id()<<std::endl;
    while(!is_exit())
    {
        //std::unique_lock<std::mutex> ul(_rlm);
        std::shared_ptr<XTask<void>> task = GetTask();
        if(task == nullptr) continue;
        ++_run_num;
        try
        {
            task->Run();
        }catch(...)
        {
            std::cerr<<"Error\n";
        }
        --_run_num;
        //if(_run_num == 0)
        //    _stop.notify_all();
    }
    //sy_out<<"End Thread:"<<std::this_thread::get_id()<<std::endl;
}


template <typename _T>
void XThread_Pool<_T>::Start()
{
    if(_thread_num <= 0)
        std::cerr<<"Thread Number Error!\n";
    if(!_ths.empty())
        std::cerr<<"Pool Start already!\n";
    //std::unique_lock<std::mutex> lock(_mtx);
    for(int i = 0;i < _thread_num;++i)
        _ths.push_back(std::make_shared<std::thread>(&XThread_Pool<_T>::Run,this));
}
template <>
void XThread_Pool<void>::Start()
{
    if(_thread_num <= 0)
        std::cerr<<"Thread Number Error!\n";
    if(!_ths.empty())
        std::cerr<<"Pool Start already!\n";
    //std::unique_lock<std::mutex> lock(_mtx);
    for(int i = 0;i < _thread_num;++i)
        _ths.push_back(std::make_shared<std::thread>(&XThread_Pool<void>::Run,this));
}


template <>
void XThread_Pool<void>::_Stop()
{
    //std::unique_lock<std::mutex> lock(_m_stop);
    //_stop.wait(lock);
    while(!_tasks.empty())
        XThread_Pool<void>::Run();
    _is_exit = true;
    _cv.notify_all();
    for(auto &th:_ths)
        th->join();
    //std::unique_lock<std::mutex> lock(_mtx);
    //_ths.clear();
}

template <typename _T>
void XThread_Pool<_T>::_Stop()
{
    //std::unique_lock<std::mutex> lock(_m_stop);
    //_stop.wait(lock);
    _is_exit = true;
    _cv.notify_all();
    for(auto &th:_ths)
        th->join();
    //std::unique_lock<std::mutex> lock(_mtx);
    //_ths.clear();
}

template <typename _T>
void XThread_Pool<_T>::_StopAt(timeval &_time)
{
    //std::unique_lock<std::mutex> lock(_m_stop);
    //_stop.wait(lock);
    timeval tmp;
    while(1)
    {
        while(!_tasks.empty())
        {
            XThread_Pool<_T>::Run();
            gettimeofday(&_timer,NULL);
        }
        gettimeofday(&tmp,NULL);
        if(tmp.tv_sec == _time.tv_sec + _timer.tv_sec &&
            tmp.tv_usec == _time.tv_usec + _timer.tv_usec )
        {
            _is_exit = true;
            _cv.notify_all();
            for(auto &th:_ths)
                th->join();
            break;
        }
        
    }
    //std::unique_lock<std::mutex> lock(_mtx);
    //_ths.clear();
}
template <>
void XThread_Pool<void>::_StopAt(timeval &_time)
{
    //std::unique_lock<std::mutex> lock(_m_stop);
    //_stop.wait(lock);
    timeval tmp;
    while(1)
    {
        while(!_tasks.empty())
        {
            XThread_Pool<void>::Run();
            gettimeofday(&_timer,NULL);
        }
        gettimeofday(&tmp,NULL);
        if(tmp.tv_sec == _time.tv_sec + _timer.tv_sec &&
            tmp.tv_usec == _time.tv_usec + _timer.tv_usec )
        {
            _is_exit = true;
            _cv.notify_all();
            for(auto &th:_ths)
                th->join();
            break;
        }
        
    }
    //std::unique_lock<std::mutex> lock(_mtx);
    //_ths.clear();
}

template <typename _T>
void XThread_Pool<_T>::Stop()
{
    _stop = true;
    _mgr = std::thread([this](){
        XThread_Pool<_T>::_Stop();
    });
    _mgr.join();
}
template <>
void XThread_Pool<void>::Stop()
{
    _stop = true;
    _mgr = std::thread([this](){
        XThread_Pool<void>::_Stop();
    });
    _mgr.join();
}


template <typename _T>
void XThread_Pool<_T>::StopAt(timeval &_time)
{
    _stop = true;
    _mgr = std::thread([this,&_time](){
        XThread_Pool<_T>::_StopAt(_time);
    });
    _mgr.join();
}
template <>
void XThread_Pool<void>::StopAt(timeval &_time)
{
    _stop = true;
    _mgr = std::thread([this,&_time](){
        XThread_Pool<void>::_StopAt(_time);
    });
    _mgr.join();
}



template <typename _T>
void XThread_Pool<_T>::AddTask(std::shared_ptr<XTask<_T>> task)
{
    std::unique_lock<std::mutex> lock(_mtx);
    task->is_exit = [this](){return XThread_Pool<_T>::is_exit();};
    _tasks.push_back(task);
    lock.unlock();
    _cv.notify_one();
}
template <>
void XThread_Pool<void>::AddTask(std::shared_ptr<XTask<void>> task)
{
    //printf("get lock\n");
    std::unique_lock<std::mutex> lock(_mtx);
    //printf("end get\n");
    task->is_exit = [this](){return XThread_Pool<void>::is_exit();};
    //printf("Push task\n");
    _tasks.push_back(task);
    lock.unlock();
    //printf("Start notify\n");
    _cv.notify_one();
}


template <typename _T>
std::shared_ptr<XTask<_T>> XThread_Pool<_T>::GetTask()
{
    std::unique_lock<std::mutex> lock(_mtx);
    if(_tasks.empty())
        _cv.wait(lock);
    if(XThread_Pool<_T>::is_exit())
        return nullptr;
    if(_tasks.empty())
        return nullptr;
    std::shared_ptr<XTask<_T>> task = _tasks.front();
    _tasks.pop_front();
    //task->is_exit = [this](){return is_exit;};
    return task;
}