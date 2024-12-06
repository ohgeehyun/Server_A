#include "pch.h"
#include "JobSerializer.h"
#include "Job.h"


void JobSerializer::Push(IJopRef job)
{
    bool flush = false;
    WRITE_LOCK
    {
       _jobQueue.push(std::move(job));
         if (_flush == false)
          flush = _flush = true;
    }
        if (flush)
            Flush();
}

void JobSerializer::Flush()
{
    while (true)
    {
        auto job = Pop();
        //!job 과 연산이같다 shared_ptr은 boolean연산에서 nullptr이면 false를 반환
        if (job == nullptr) {
            return;
        }
        job->Execute();
    }
}

IJopRef JobSerializer::Pop()
{
    WRITE_LOCK;
    {
        if (_jobQueue.size() == 0)
        {
            _flush = false;
            return nullptr;
        }
        
        auto job = std::move(_jobQueue.front());
        _jobQueue.pop();
        return job;
    }

}


