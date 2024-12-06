#pragma once
#include "Job.h"

class JobSerializer 
{
public:
     // 단일 인자용 템플릿 메서드
     template <typename R, typename Arg>
     void Push(std::function<R(Arg)> job, Arg arg);

     template <typename R, typename ...Args>
     void Push(std::function<R(Args...)> job, Args&&... args);

     void Push(IJopRef job);
     
     void Flush();

     IJopRef Pop();
    

private:
    Queue<IJopRef> _jobQueue;
    USE_LOCK;
    bool _flush = false;
};

template<typename R, typename Arg>
inline void JobSerializer::Push(std::function<R(Arg)> job, Arg arg)
{
    IJopRef jobInstance = std::make_shared<Job<R, Arg>>(job, std::make_tuple(arg));
    Push(jobInstance);
}

template<typename R, typename ...Args>
inline void JobSerializer::Push(std::function<R(Args...)> job, Args && ...args)
{
    IJopRef jobInstance = std::make_shared<Job<R, Args...>>(job, std::forward_as_tuple(std::forward<Args>(args)...));
    Push(jobInstance);
}



