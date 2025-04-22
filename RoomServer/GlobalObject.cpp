#include "pch.h"
#include "GlobalObject.h"
#include "RedisConnection.h"
#include "NetAddress.h"
std::shared_ptr<RedisConnection> GRedisConnection = nullptr;

class ServerGlobal
{
public:
    ServerGlobal()
    {
        //초기화는 main의 시작부분에서 해준다 링커순서에 의 해 main()함수에서 초기화.
    }

    ~ServerGlobal()
    {
       
    }
}GServerGlobal;