#include "pch.h"
#include "RedisConnection.h"
#include <hiredis/hiredis.h>
#include <sw/redis++/redis++.h>
#include "RoomManager.h"
#include "DataManager.h"
RedisConnection::RedisConnection()
{
    HashMap<string, ServerConfigData> dict = DataManager::GetInstance().GetServerConfigDict();
    AsyncConnect(dict["database"].redisData.host, dict["database"].redisData.port);
}

RedisConnection::~RedisConnection()
{
    // TODO : 연결 끊어주기
}

void RedisConnection::AsyncConnect(const std::string& host, int32 port)
{
    _context = redisAsyncConnect(host.c_str(), port);
    
    if (_context == nullptr) {
        std::cerr << "Error: can't allocate redis context" << '\n';
    };

    _eventBase = event_base_new();
    if (redisLibeventAttach(_context, _eventBase) != REDIS_OK) {
        std::cerr << "Failed to attach Redis context to libevent" << std::endl;
    }

    // redisAsyncSetConnectCallback에 래퍼 콜백 설정
    redisAsyncSetConnectCallback(_context, [](const redisAsyncContext* context, int status) {
        // 래퍼 콜백에서 실제 멤버 함수 호출
        RedisConnection* connection = static_cast<RedisConnection*>(context->data);
        connection->OnConnect(context, status);
    });
    
}

void RedisConnection::RunEventLoop()
{
    if (_eventBase)
        event_base_loop(_eventBase, EVLOOP_NONBLOCK);
}

void RedisConnection::OnConnect(const redisAsyncContext* context, int status)
{
    if (status != REDIS_OK) {
        std::cerr << "Connection failed: " << context->errstr << '\n';
        return;
    }
    else {
        std::cout << "Connected to Redis server successfully!" << '\n';
    }

    OnAuthenticate(const_cast<redisAsyncContext*>(context));
}

void RedisConnection::OnAuthenticate(redisAsyncContext* context)
{
    HashMap<string, ServerConfigData> dict = DataManager::GetInstance().GetServerConfigDict();
    const std::string password = dict["database"].redisData.auth;

    // 비동기적으로 AUTH 명령을 보내 Redis 서버에 인증 요청
    redisAsyncCommand(context, [](redisAsyncContext* context, void* reply, void* privdata) {
        // 인증 완료 후의 콜백 처리
        if (reply == nullptr)
            std::cerr << "Authentication failed!" << '\n';

        // 인증 성공 후 추가 작업 굳이 또 다른 함수를 호출할 작업까진 연결성공이라면 지금은 없지만 나중을 위하여 작성
        RedisConnection* conn = static_cast<RedisConnection*>(context->data);
        conn->PostAuthTask(context);  // 인증 후 작업을 위한 함수 호출

    }, nullptr, "AUTH %s", password.c_str());
}

void RedisConnection::PostAuthTask(redisAsyncContext* context)
{
    std::cout << "Authentication successful!" << '\n';
    
    redisAsyncCommand(context, [](redisAsyncContext* context, void* reply, void* privdata) {
        
        // void* 타입인 reply를 redisReply*로 캐스팅
        redisReply* redisReplyPtr = static_cast<redisReply*>(reply);
        if (reply == nullptr) {
            std::cerr << "PING failed!" << '\n';
        }
        else {
            std::cout << "PING successful! Response:"  << redisReplyPtr->str<<'\n';  // Redis가 정상적으로 응답하면 성공 메시지 출력
        }
        RedisConnection* conn = static_cast<RedisConnection*>(context->data);
        conn->OnCheckToCreateData(context); // 방 연결 이후 Redis 에 등록된 방에 따른 방 생성
    }, nullptr, "PING");
}

void RedisConnection::OnCheckToCreateData(redisAsyncContext* context)
{

    redisAsyncCommand(context, [](redisAsyncContext* context, void* reply, void* privdata) {
        redisReply* res = (redisReply*)reply;

        if (res == nullptr) {
            std::cerr << "Error in SCAN response." << std::endl;
            return;
        }
        RedisConnection* conn = static_cast<RedisConnection*>(context->data);
        conn->GetRoom(context, res); // 응답을 처리하는 함수 호출
    },nullptr, "SCAN 0 MATCH room:[0-9]*");
}

void RedisConnection::GetRoom(redisAsyncContext* context, redisReply* res)
{
 
    // SCAN 결과 처리: 첫 번째 요소는 커서 값이고, 두 번째 요소부터 키 목록
    if (res->type == REDIS_REPLY_ARRAY) {
        // 첫 번째 요소는 커서 값 (처리하지 않음)
        std::string cursor = res->element[0]->str;

        for (size_t i = 1; i < res->elements; i++) {
            // 만약 element[i]가 REDIS_REPLY_ARRAY라면 그 안에서 또 키들을 찾아야 합니다.
            if (res->element[i]->type == REDIS_REPLY_ARRAY) {
                // 키 목록이 배열로 감싸져 있으면, 그 안에서 문자열들을 출력합니다.
                for (size_t j = 0; j < res->element[i]->elements; j++) {
                    if (res->element[i]->element[j] != nullptr && res->element[i]->element[j]->type == REDIS_REPLY_STRING) {
                        std::string key(res->element[i]->element[j]->str, res->element[i]->element[j]->len);
                        std::cout << "key value: " << key << std::endl;

                        //여기에 이제 key 값을 받아서 CreateRoom함수에  jsondata넘겨주고 파싱해야할듯
                        CreateRoom(context,res,key);
                    }
                    else {
                        std::cerr << "Error: Invalid element at nested index " << j << std::endl;
                    }
                }
            }

            // 커서가 0이 아니면 다음 SCAN 명령을 호출하여 계속 반복
            if (cursor != "0") {
                std::cout << "Continuing scan with cursor: " << cursor << std::endl;
                redisAsyncCommand(context, [](redisAsyncContext* context, void* reply, void* privdata) {
                    redisReply* res = (redisReply*)reply;
                    RedisConnection* conn = static_cast<RedisConnection*>(context->data);
                    conn->GetRoom(context, res);
                }, nullptr, "SCAN %s MATCH room:[0-9]*", cursor.c_str());
            }
            else {
                std::cout << "Scan complete." << std::endl;
            }
        }    
    }
  
}

void RedisConnection::CreateRoom(redisAsyncContext* context, redisReply* res, string room)
{
    redisAsyncCommand(context, [](redisAsyncContext* context, void* reply, void* privdata) {
        redisReply* res = (redisReply*)reply;
        RedisConnection* conn = static_cast<RedisConnection*>(context->data);

        const std::string key = static_cast<const char*>(privdata);

        if (res->type == REDIS_REPLY_STRING) {
            std::string json_data(res->str, res->len);
            std::cout << "Received JSON for key " << key << ": " << json_data << std::endl;

            // JSON 파싱
            try {
                nlohmann::json jsonparse = nlohmann::json::parse(json_data);  // JSON 파싱

                // 파싱된 JSON 데이터에서 필요한 값 추출
                int id = jsonparse["id"].get<int>();
                std::string name = jsonparse["name"].get<std::string>();
                std::string password = jsonparse["password"].get<std::string>();
                std::string nickname = jsonparse["rootUser"].get<std::string>();
                bool pwdYn = jsonparse["pwdYn"].get<bool>();

                RoomManager::GetInstance().Add(1,name, password,id, nickname);
            }
            catch (const nlohmann::json::exception& e) {
                std::cerr << "Error parsing JSON for key " << key << ": " << e.what() << std::endl;
            }
        }
        else {
            std::cerr << "Expected a string value for key: " << key << std::endl;
        }

    }, (void*)room.c_str(), "GET %s",room.c_str());
}


