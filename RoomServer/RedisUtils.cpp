#include "pch.h"
#include "RedisUtils.h"

void RedisUtils::replyResponseHandler(void* reply,const char* log)
{
    redisReply* r = (redisReply*)reply;  // redisReply로 캐스팅
    if (r->type == REDIS_REPLY_STATUS) 
    {
        // "OK"가 반환되었을 때
        std::cout << log << " : "  << r->str << std::endl;
    }
    else if (r->type == REDIS_REPLY_ERROR) 
    {
        // 오류 응답 처리
        std::cout << log << "Error: " << r->str << std::endl;
    }
    else if (r->type == REDIS_REPLY_INTEGER)
    {
        //integer반환처리
        std::cout << log << " : "  << r->integer << std ::endl;
    }
    else {
        std::cout << log << " : "  <<"Unexpected response type" << std::endl;
    }
}

void RedisUtils::testGetvalue(void* reply)
{
   redisReply* r = (redisReply*)reply;  // redisReply로 캐스팅
   if (r->type == REDIS_REPLY_STRING) {
       std::cout << "Fetched value: " << r->str << std::endl; // 값이 문자열이라면 출력
   }
   else {
       std::cout << "Error: Unexpected Redis reply type" << std::endl;
   }
}
