#include "pch.h"
#include "RedisPubSubConnection.h"
#include "RedisUtils.h" 
#include "Utils.h"
#include <nlohmann/json.hpp> 

RedisPubSubConnection::RedisPubSubConnection()
{
    ConnectAsync([](bool success, const std::string& msg) {
        if (success)
        {
            std::cout << "redis connect success" << std::endl;
        }
        else
        {
            std::cerr << "Redis connect failed : " << msg << std::endl;
        }
    });
}

RedisPubSubConnection::~RedisPubSubConnection()
{
    if (_context)
        redisAsyncFree(_context);
}

void RedisPubSubConnection::ConnectAsync(ConnectCallback callback)
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
        RedisPubSubConnection* self = static_cast<RedisPubSubConnection*>(c->data);
        self->OnConnected(c, status);
    });
}

void RedisPubSubConnection::OnConnected(const redisAsyncContext* context, int status)
{
    if (status != REDIS_OK) {
        _connectCallback(false, context->errstr);
        return;
    }

    const std::string password = ServerConfig["database"].redisData.auth;
    Authenticate(const_cast<redisAsyncContext*>(context), password);
}

void RedisPubSubConnection::Authenticate(redisAsyncContext* context, const std::string& password)
{
    redisAsyncCommand(context, [](redisAsyncContext* c, void* reply, void* privdata) {
        RedisPubSubConnection* self = static_cast<RedisPubSubConnection*>(c->data);
        self->OnAuthenticated(c, reply);
    }, nullptr, "AUTH %s", password.c_str());
}

void RedisPubSubConnection::OnAuthenticated(redisAsyncContext* context, void* reply)
{
    if (reply == nullptr) {
        _connectCallback(false, "Redis AUTH failed (no reply)");
        return;
    }

    _connectCallback(true, "");
    std::cout << "[Redis] Authentication success.\n";

    RegisterRoomServer();  //서버 정보 등록 시작
}

void RedisPubSubConnection::RegisterRoomServer()
{
    RedisUtils::RAsyncCommandCallback(_context,
        [this](redisReply* reply)
        {
        if (reply && reply->type == REDIS_REPLY_INTEGER)
        {
            _roomId = static_cast<int32_t>(reply->integer);
            SaveRoomServerInfo();          // 정보 저장
            PublishRoomServerRegister();   // 채널 발행
        }
        else
        {
            std::cerr << "[Redis] Failed to get room_id from INCR.\n";
        }
        },
        "INCR room_server_id_seq");
}

void RedisPubSubConnection::SaveRoomServerInfo()
{
    std::string key = "room_servers:" + std::to_string(_roomId);

    RedisUtils::RAsyncCommand(_context,
        "HMSET %s ip %s port %d status %s",
        key.c_str(),
        Utils::WstringToUtf8(_serverIp).c_str(),
        _serverPort,
        "available");

    StartHeartbeat();
}

void RedisPubSubConnection::PublishRoomServerRegister()
{
    nlohmann::json data = {
       {"room_id", _roomId},
       {"ip", Utils::WstringToUtf8(_serverIp)},
       {"port", _serverPort}
    };

    cout << Utils::WstringToUtf8(_serverIp) << endl;

    std::string msg = data.dump(); // JSON → string

    RedisUtils::RAsyncCommand(_context,
        "PUBLISH channel:room_servers %s",
        msg.c_str());
}

void RedisPubSubConnection::RunEventLoopOnce()
{
    if (_eventBase)
        event_base_loop(_eventBase.get(), EVLOOP_NONBLOCK);
}

void RedisPubSubConnection::SendHeartbeat()
{
    RedisUtils::RAsyncCommand(_context,
        "EXPIRE room_servers:%d 5", _roomId);
}

void RedisPubSubConnection::StartHeartbeat()
{
    DoTimer(3000, [&]() {
        //Redis에 생존 신고
        SendHeartbeat();
        cout << "Send Redis room_server Heart Beat!" << '\n';
        // 다음 하트비트 예약
        StartHeartbeat();
    });
 
}