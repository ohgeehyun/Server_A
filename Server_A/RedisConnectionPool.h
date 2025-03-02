#pragma once

/*--------------------------------------
            RedisConnectionPool
---------------------------------------*/
//redis++의 경우 단일 스레드에서 돌아가게 설계가 되어있다
//일단 멀티스레드 환경에서 여러스레드로 관리를 해야할 수도있으니 Pool을 작성은 하는데 일단은 사용하지 않음.
class RedisConnectionPool
{
public:
    RedisConnectionPool(int32 ConnCount);
    ~RedisConnectionPool();


private:
    bool Connect(int32 ConnCount);

private:
    USE_LOCK;
    Vector<RedisConnectionRef> _Pool;
    Atomic<int32> _connectCount = 0;
    Atomic<int32> _workerCount = 0;
};

