#include "pch.h"
#include "RedisManager.h"

/*------------------------------*/
//         RedisManager
/*------------------------------*/

RedisManager::RedisManager()
{
    _commandNode = new RedisConnection();
    _pubsubNode = new RedisPubSubConnetion();
}

RedisManager::~RedisManager()
{
    delete _commandNode;
    delete _pubsubNode;
}

void RedisManager::CommandNode_RunEventLoopOnce()
{
    WRITE_LOCK
    {
          _commandNode->RunEventLoopOnce();
    }
}

void RedisManager::PubSubNode_RunEventLoopOnce()
{
    _pubsubNode->RunEventLoopOnce();
}
