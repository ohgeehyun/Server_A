#pragma once

/*------------------------
        GlobalQueue
-------------------------*/
class GlobalQueue
{
public:
    GlobalQueue();
    ~GlobalQueue();

    void             Push(JobQueueRef jobQueue);
    JobQueueRef      Pop();

private:
    LockQueue<JobQueueRef> _jobQueues;
};


/*------------------------
        GlobalDBQueue
-------------------------*/
class GlobalDBQueue
{
public:
    GlobalDBQueue();
    ~GlobalDBQueue();

    void             Push(JobQueueRef jobQueue);
    JobQueueRef      Pop();

private:
    LockQueue<JobQueueRef> _jobQueues;
};

