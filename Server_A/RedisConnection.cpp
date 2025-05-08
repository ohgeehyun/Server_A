#include "pch.h"
#include "RedisConnection.h"
#include "NetAddress.h"



RedisConnection::RedisConnection()
{
    ConnectAsync([](bool success, const std::string& msg) {
        if (success) 
        {
            std::cout << "redis connect success"<<endl;
        }
        else
        {
            std::cerr << "Redis connect failed : "<< msg <<endl;
        }
    });
}

RedisConnection::~RedisConnection()
{
    if (_context)
        redisAsyncFree(_context);
}

void RedisConnection::ConnectAsync(ConnectCallback callback)
{
    _connectCallback = std::move(callback);

    _context = redisAsyncConnect(ServerConfig["database"].redisData.host.c_str(), ServerConfig["database"].redisData.port);
    if (_context == nullptr || _context->err) {
        std::string errMsg = _context ? _context->errstr : "null context";
        _connectCallback(false, "Redis connect failed: " + errMsg);
        return;
    }

    _context->data = this;
    _eventBase.reset(event_base_new());

    if (redisLibeventAttach(_context, _eventBase.get()) != REDIS_OK) {
        _connectCallback(false, "Failed to attach libevent");
        return;
    }

    redisAsyncSetConnectCallback(_context, [](const redisAsyncContext* c, int status) {
        RedisConnection* self = static_cast<RedisConnection*>(c->data);
        self->OnConnected(c, status);
    });
}

void RedisConnection::OnConnected(const redisAsyncContext* context, int status)
{
    if (status != REDIS_OK) {
        _connectCallback(false, context->errstr);
        return;
    }

    // 연결 성공 후 인증 시작
    const std::string password = ServerConfig["database"].redisData.auth; // 필요시 멤버로 저장
    Authenticate(const_cast<redisAsyncContext*>(context), password);
}

void RedisConnection::Authenticate(redisAsyncContext* context, const std::string& password)
{
    redisAsyncCommand(context, [](redisAsyncContext* c, void* reply, void* privdata) {
        RedisConnection* self = static_cast<RedisConnection*>(c->data);
        self->OnAuthenticated(c, reply);
    }, nullptr, "AUTH %s", password.c_str());
}

void RedisConnection::OnAuthenticated(redisAsyncContext* context, void* reply)
{
    if (reply == nullptr) {
        _connectCallback(false, "Redis AUTH failed (no reply)");
        return;
    }

    std::cout << "[Redis] Authentication success.\n";
}


void RedisConnection::RunEventLoopOnce()
{
    if (_eventBase)
        event_base_loop(_eventBase.get(), EVLOOP_NONBLOCK);
}