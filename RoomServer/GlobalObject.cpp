#include "pch.h"
#include "GlobalObject.h"
#include "RedisConnection.h"
#include "NetAddress.h"
#include "RoomManager.h"
std::shared_ptr<RedisConnection> GRedisConnection = nullptr;
std::shared_ptr<RoomManager> GRoomManager;
class ServerGlobal
{
public:
    ServerGlobal()
    {
       
    }

    ~ServerGlobal()
    {
       
    }
}GServerGlobal;