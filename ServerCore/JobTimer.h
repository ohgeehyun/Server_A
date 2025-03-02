#pragma once

struct JobData
{
    JobData(weak_ptr<JobQueue> owner, JobRef  job) : owner(owner), job(job) {};

    weak_ptr<JobQueue> owner;
    JobRef             job;
};

struct TimerItem
{
    bool operator < (const TimerItem& other) const
    {
        return exeuteTick > other.exeuteTick;
    };

    uint64 exeuteTick = 0;//실행 되어야할 틱
    JobData* jobData = nullptr;
};

/*------------------
      jobTimer
------------------*/
class JobTimer
{
public:
    void Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job);
    void Distribute(uint64 now);
    void Clear();


private:
    USE_LOCK;
    PriorityQueue<TimerItem>    _items;
    Atomic<bool>                _distributing = false;
};

