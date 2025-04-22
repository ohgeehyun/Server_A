#pragma once
#include "DataManager.h"
#include <atomic>
#include "JobQueue.h"

class RedisConnection : public JobQueue
{
public:
    using ConnectCallback = std::function<void(bool success, const std::string& errorMsg)>;

    RedisConnection();
    ~RedisConnection();

    void ConnectAsync(ConnectCallback callback);
    void RunEventLoopOnce(); // 비동기 루프 1회 실행
    redisAsyncContext* GetContext() const { return _context; }

private:
    void OnConnected(const redisAsyncContext* context, int status);
    void Authenticate(redisAsyncContext* context, const std::string& password);
    void OnAuthenticated(redisAsyncContext* context, void* reply);

    void RegisterRoomServer();
    void SaveRoomServerInfo();
    void PublishRoomServerRegister();
    void SendHeartbeat();
    void StartHeartbeat();

private:
    struct EventBaseDeleter {
        void operator()(event_base* base) const { if (base) event_base_free(base); }
    };

    std::unique_ptr<event_base, EventBaseDeleter> _eventBase;
    redisAsyncContext* _context = nullptr;
    ConnectCallback _connectCallback;
    std::atomic_bool _isDispatching = false;

    HashMap<string, ServerConfigData> ServerConfig = DataManager::GetInstance().GetServerConfigDict();

    int32 _roomId = -1;
    std::wstring _serverIp = GNetAddress->GetIpAdress();  // 추후 Config에서 가져올 수도 있음
    int32 _serverPort = GNetAddress->GetPort();           // 추후 Config에서 가져올 수도 있음
};
