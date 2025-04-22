#include "pch.h"
#include "RedisUtils.h"

void RedisUtils::ReplyResponseHandler(void* reply, std::string_view log)
{
    redisReply* r = static_cast<redisReply*>(reply);

    if (!r)
    {
        std::cerr << log << " : null reply received" << std::endl;
        return;
    }

    switch (r->type)
    {
    case REDIS_REPLY_STATUS:
        std::cout << log << " : " << r->str << std::endl;
        break;
    case REDIS_REPLY_ERROR:
        std::cerr << log << " Error: " << r->str << std::endl;
        break;
    case REDIS_REPLY_INTEGER:
        std::cout << log << " : " << r->integer << std::endl;
        break;
    default:
        std::cerr << log << " : Unexpected response type: " << r->type << std::endl;
        break;
    }
}

void RedisUtils::TestGetValue(void* reply)
{
    redisReply* r = static_cast<redisReply*>(reply);

    if (!r)
    {
        std::cerr << "TestGetValue : null reply received" << std::endl;
        return;
    }

    if (r->type == REDIS_REPLY_STRING)
    {
        std::cout << "Fetched value: " << r->str << std::endl;
    }
    else
    {
        std::cerr << "Error: Unexpected Redis reply type " << r->type << std::endl;
    }
}