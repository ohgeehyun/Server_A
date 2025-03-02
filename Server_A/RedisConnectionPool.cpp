#include "pch.h"
#include "RedisConnection.h"
#include "RedisConnectionPool.h"
/*--------------------------------------
            RedisConnectionPool
---------------------------------------*/
RedisConnectionPool::RedisConnectionPool(int32 ConnCount)
{
    Connect(3);
}

RedisConnectionPool::~RedisConnectionPool()
{
}


bool RedisConnectionPool::Connect(int32 ConnCount)
{
    for (int32 i = 0; i < ConnCount; i++)
    {
        RedisConnectionRef redis;
        redis->AsyncConnect("",1234);
        _Pool.push_back(redis);
        _connectCount.fetch_add(1);
    }
    return true;
}


