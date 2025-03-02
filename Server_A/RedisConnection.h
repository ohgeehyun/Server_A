#pragma once


class RedisConnection
{
public:
    RedisConnection();
    ~RedisConnection();

    using ConnectCallback = std::function<void(const redisAsyncContext*, int)>;

    void AsyncConnect(const std::string& host, int32 port);
    void RunEventLoop();
    bool GetIsDispatching() { return isDispatching; }
    redisAsyncContext* GetContext() { return  _context; };
   

private:
    bool isDispatching = false;
    redisAsyncContext* _context;
    struct event_base* _eventBase;
    ConnectCallback _connectCallback;
private:
    void OnConnect(const redisAsyncContext* context, int status);
    void OnAuthenticate(redisAsyncContext* context);
    void PostAuthTask(redisAsyncContext* context);
    void OnCheckToCreateData(redisAsyncContext* context);
    void GetRoom(redisAsyncContext* context,redisReply* res);
    void CreateRoom(redisAsyncContext* context, redisReply* res, string room);
};


