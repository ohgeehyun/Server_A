#include "pch.h"
#include "RedisManager.h"

/*------------------------------*/
//         RedisManager
/*------------------------------*/

RedisManager::RedisManager()
{
    _commandNode = make_shared<RedisConnection>();
    _pubsubNode = make_shared<RedisPubSubConnection>();
}

RedisManager::~RedisManager()
{
    _commandNode = nullptr;
    _pubsubNode = nullptr;
}

void RedisManager::CommandNode_RunEventLoopOnce()
{
   _commandNode->RunEventLoopOnce();
}

void RedisManager::PubSubNode_RunEventLoopOnce()
{
    _pubsubNode->RunEventLoopOnce();
}

