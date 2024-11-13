#include "pch.h"
#include "ThreadManager.h"

ThreadManager::ThreadManager()
{
    InitTLS();
}

ThreadManager::~ThreadManager()
{
    Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
    LockGuard guard(_lock);
    _threads.push_back(thread([=]()
    {
        InitTLS();
        callback();
        DestroyTLS();
    }));

}

void ThreadManager::Join()
{
    for (thread& t : _threads)
    {
        if (t.joinable()) //thread가 join가능한지 확인
            t.join();
    }
    _threads.clear();
}

void ThreadManager::InitTLS()
{
    static Atomic<uint32> SThread = 1;
    LThreadId = SThread.fetch_add(1); //메인 스레드 +1
}

void ThreadManager::DestroyTLS()
{
    // TODO : 현재는 딱히 thread_local에서 제거할 것이 없음. 나중에 필요하면 작성
}