#include "pch.h"
#include "GlobalObject.h"
#include "MysqlConnectionPool.h"
#include "RedisConnection.h"
MysqlConnectionPool* GDBConnectionPool = nullptr;
RedisConnection* GRedisConnection = nullptr;
class ServerGlobal
{
public:
    ServerGlobal()
    {
        //초기화는 main의 시작부분에서 해준다 링커순서에 의 해 pch에서 할시 초기화를 못하는 경우가 있다.
    }

    ~ServerGlobal()
    {
        delete GDBConnectionPool;
        delete GRedisConnection;
    }
}GServerGlobal;