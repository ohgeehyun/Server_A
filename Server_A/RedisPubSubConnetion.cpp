#include "pch.h"
#include "RedisPubSubConnetion.h"
#include "GameSessionManager.h"
#include "NetAddress.h"

RedisPubSubConnetion::RedisPubSubConnetion()
{
    ConnectAsync([](bool success, const std::string& msg) {
        if (success)
        {
            std::cout << "[Redis Pub/Sub] redis connect success" << endl;
        }
        else
        {
            std::cerr << "[Redis Pub/Sub] Redis connect failed : " << msg << endl;
        }
    });
}

RedisPubSubConnetion::~RedisPubSubConnetion()
{
    if (_context)
        redisAsyncFree(_context);
}

void RedisPubSubConnetion::ConnectAsync(ConnectCallback callback)
{
    _connectCallback = std::move(callback);

    _context = redisAsyncConnect(ServerConfig["database"].redisData.host.c_str(), ServerConfig["database"].redisData.port);
    if (_context == nullptr || _context->err) {
        std::string errMsg = _context ? _context->errstr : "null context";
        _connectCallback(false, "[Redis Pub/Sub] Redis connect failed: " + errMsg);
        return;
    }

    _context->data = this;
    _eventBase.reset(event_base_new());

    if (redisLibeventAttach(_context, _eventBase.get()) != REDIS_OK) {
        _connectCallback(false, "[Redis Pub/Sub] Failed to attach libevent");
        return;
    }

    redisAsyncSetConnectCallback(_context, [](const redisAsyncContext* c, int status) {
        RedisPubSubConnetion* self = static_cast<RedisPubSubConnetion*>(c->data);
        self->OnConnected(c, status);
    });
}

void RedisPubSubConnetion::RunEventLoopOnce()
{
    if (_eventBase)
        event_base_loop(_eventBase.get(), EVLOOP_NONBLOCK);
}

void RedisPubSubConnetion::OnConnected(const redisAsyncContext* context, int status)
{
    if (status != REDIS_OK) {
        _connectCallback(false, context->errstr);
        return;
    }

    // 연결 성공 후 인증 시작
    const std::string password = ServerConfig["database"].redisData.auth; // 필요시 멤버로 저장
    Authenticate(const_cast<redisAsyncContext*>(context), password);
}

void RedisPubSubConnetion::Authenticate(redisAsyncContext* context, const std::string& password)
{
    redisAsyncCommand(context, [](redisAsyncContext* c, void* reply, void* privdata) {
        RedisPubSubConnetion* self = static_cast<RedisPubSubConnetion*>(c->data);
        self->OnAuthenticated(c, reply);
    }, nullptr, "AUTH %s", password.c_str());
}

void RedisPubSubConnetion::OnAuthenticated(redisAsyncContext* context, void* reply)
{
    if (reply == nullptr) {
        _connectCallback(false, "[Redis Pub/Sub] Redis AUTH failed (no reply)");
        return;
    }

    RedisPubSubConnetion* self = static_cast<RedisPubSubConnetion*>(context->data);

    _connectCallback(true, "");
    std::cout << "[Redis Pub/Sub] Authentication success.\n";
    //auth 인증 완료후 Redis에 올라와 있는 RoomServer정보 가저오는 함수 호출
    OnRoomServerInfoFetched([self](const std::vector<NetAddress>& servers)
    {
        for (auto& s : servers)
        {
            std::wcout << L"[Redis] Room Server: " << s.GetIpAdress() << " : " << s.GetPort() << "\n";
            //전역 클라이언트 서비스에서 클라이언트 추가 호출
            GPClientService->Add_ClientInfo(s);
        }
        //구독
        self->SubscribeRoomServerChannel();
        self->SubscribeUserInfoChannel();
        ASSERT_CRASH(GPClientService->Start());
    });
}

void RedisPubSubConnetion::LoadRoomServers(std::function<void(const std::vector<NetAddress>&)> callback)
{
    redisAsyncCommand(_context,
        [](redisAsyncContext* c, void* r, void* privdata)
    {

        redisReply* reply = static_cast<redisReply*>(r);

        if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY)
            return;

        std::vector<std::string> keys;
        for (size_t i = 0; i < reply->element[1]->elements; ++i)
        {
            keys.push_back(reply->element[1]->element[i]->str);
        }

        RedisPubSubConnetion* self = static_cast<RedisPubSubConnetion*>(c->data);
        self->FetchRoomServerInfos(keys, std::move(*static_cast<std::function<void(std::vector<NetAddress>)>*>(privdata)));
        delete static_cast<std::function<void(std::vector<NetAddress>)>*>(privdata);

    }, new std::function<void(std::vector<NetAddress>)>(std::move(callback)),
        "SCAN 0 MATCH room_servers:*");
}

void RedisPubSubConnetion::FetchRoomServerInfos(const std::vector<std::string>& keys, std::function<void(const std::vector<NetAddress>&)> callback)
{
    struct FetchContext
    {
        std::shared_ptr<std::vector<NetAddress>> result = std::make_shared<std::vector<NetAddress>>();
        std::shared_ptr<std::atomic<int32_t>> pending;
        std::function<void(const std::vector<NetAddress>&)> finalCallback;
    };

    auto ctx = std::make_shared<FetchContext>();
    ctx->pending = std::make_shared<std::atomic<int32_t>>(static_cast<int32_t>(keys.size()));
    ctx->finalCallback = std::move(callback);

    for (const auto& key : keys)
    {
        RedisUtils::RAsyncCommandCallback(_context,
            [ctx](redisReply* reply)
        {
            if (reply && reply->type == REDIS_REPLY_ARRAY)
            {
                std::string ip;
                uint16_t port = 0;
                std::string status;

                for (size_t i = 0; i + 1 < reply->elements; i += 2)
                {
                    std::string field = reply->element[i]->str;
                    std::string value = reply->element[i + 1]->str;

                    if (field == "ip") ip = value;
                    else if (field == "port") port = static_cast<uint16_t>(std::stoi(value));
                    else if (field == "status") status = value;
                }

                if (!ip.empty() && port != 0 && status == "available")
                {
                    ctx->result->emplace_back(Utils::Utf8ToWstring(ip), port);
                }
            }

            if (--(*ctx->pending) == 0)
            {
                ctx->finalCallback(*ctx->result);
            }
        }, "HGETALL %s", key.c_str());
    }
}

void RedisPubSubConnetion::OnRoomServerInfoFetched(std::function<void(const std::vector<NetAddress>&)> callback)
{
    RedisUtils::RAsyncCommandCallback(_context,
        [this, callback = std::move(callback)](redisReply* reply)
    {
        std::vector<std::string> keys;

        if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements == 2)
        {
            redisReply* keysReply = reply->element[1];
            for (size_t i = 0; i < keysReply->elements; ++i)
            {
                keys.emplace_back(keysReply->element[i]->str);
            }
        }

        if (keys.empty())
        {
            callback({});
            return;
        }

        FetchRoomServerInfos(keys, callback);
    }, "SCAN 0 MATCH room_servers:*");
}

void RedisPubSubConnetion::SubscribeRoomServerChannel()
{
    redisAsyncCommand(_context, [](redisAsyncContext* c, void* reply, void* privdata)
    {
        if (!reply) return;

        redisReply* r = static_cast<redisReply*>(reply);

        if (r->type != REDIS_REPLY_ARRAY || r->elements < 3)
            return;

        //c스타일 문법에서는 포인터와 string은 비교시 포인터 비교이다. string으로 변환후 비교 또는 strcmp함수를 사용해야함.
        std::string msgType = r->element[0]->str;

        if (msgType == "subscribe")
            cout << "[Redis Pub/Sub] room:servers subscribe complete" << endl;

        //요소가 3개인지 검사 
        if (msgType == "message")
        {
            //Redis에서 subscribe 의 반환은 배열에 프로토콜 형식,채널명,메세지(실제 데이터)기때문에 배열의2번
            std::string payload = r->element[2]->str;

            try
            {
                auto jsonData = nlohmann::json::parse(payload);
                std::string ip = jsonData["ip"];
                uint16_t port = jsonData["port"];
                //int32_t roomId = jsonData["room_id"];

                NetAddress addr(Utils::Utf8ToWstring(ip), port);

                // 여기서 addr를 사용해서 서버 리스트에 추가하든, 핸들러 호출하든 하면 됨
                std::wcout << L"[Redis] SUB : RoomServer RoomServer Addr: " << Utils::Utf8ToWstring(ip) << " : " << port << std::endl;

                GPClientService->AddClientInfoAndStart(addr);
            }
            catch (const std::exception& e)
            {
                std::cerr << "[Redis] JSON parsing failed: " << e.what() << std::endl;
            }
        }
    }, nullptr, "SUBSCRIBE channel:room_servers");
}

void RedisPubSubConnetion::SubscribeUserInfoChannel()
{
    redisAsyncCommand(_context, [](redisAsyncContext* c, void* reply, void* privdata)
    {
        if (!reply) return;

        redisReply* r = static_cast<redisReply*>(reply);
        if (r->type != REDIS_REPLY_ARRAY || r->elements < 3)
            return;

        std::string msgType = r->element[0]->str;

        if (msgType == "subscribe")
        {
            std::cout << "[Redis Pub/Sub] SUBSCRIBED: __keyevent@0__:expired" << std::endl;
            return;
        }

        if (msgType == "message")
        {
            std::string expiredKey = r->element[2]->str;

            // 키 이름이 user_info: 로 시작하는지 확인
            const std::string prefix = "user_info:";
            if (expiredKey.rfind(prefix, 0) == 0)
            {
                std::string userId = expiredKey.substr(prefix.length());
                std::cout << "[Redis] user_info expired for user_id: " << userId << std::endl;

                // 세션 정리 로직
                if (GGameSessionManager->FindWaitSession(userId))
                {
                    GGameSessionManager->SessionDelete(userId);
                    std::cout << "[Redis] User " << userId << " removed due to TTL expiration\n";
                }
            }
        }

    }, nullptr, "SUBSCRIBE __keyevent@0__:expired");
}