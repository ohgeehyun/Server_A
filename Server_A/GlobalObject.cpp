#include "pch.h"
#include "GlobalObject.h"
#include "MysqlConnectionPool.h"
#include "Service.h"
#include "RedisConnection.h"
MysqlConnectionPool* GDBConnectionPool = nullptr;
RedisConnection* GRedisConnection = nullptr;

std::shared_ptr<ClientService> GPClientService;
class ServerGlobal
{
public:
    ServerGlobal()
    {
        //초기화는 main의 시작부분에서 해준다. 
        //링커순서에 의 해 pch에서 할시 초기화를 못하는 경우가 있다.
        //해결할려고 해보았지만 ... 계속해서 초기화 순서가 어긋나서 일단은 Main함수에서 진행
    }

    ~ServerGlobal()
    {
        delete GDBConnectionPool;
        delete GRedisConnection;
    }
}GServerGlobal;