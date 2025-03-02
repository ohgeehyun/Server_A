#pragma once
class RedisUtils
{
public:
    static void replyResponseHandler(void* reply,const char* log);
    static void testGetvalue(void* reply);
};

