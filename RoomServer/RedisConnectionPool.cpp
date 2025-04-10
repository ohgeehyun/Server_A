#include "pch.h"
#include "RedisConnection.h"
#include "RedisConnectionPool.h"
/*--------------------------------------
            RedisConnectionPool
---------------------------------------*/
RedisConnectionPool::RedisConnectionPool(int32 ConnCount)
{
    Connect(ConnCount);
}

RedisConnectionPool::~RedisConnectionPool()
{
}


bool RedisConnectionPool::Connect(int32 ConnCount)
{
    for (int32 i = 0; i < ConnCount; i++)
    {
        RedisConnectionRef redis;
        _Pool.push_back(redis);
        _connectCount.fetch_add(1);
    }
    return true;
}


