#pragma once
#include "RedisUtils.h"
/*------------------------------*/
//      RedisManager
// Pub/Sub용 컨텍스트 하나 일반 command용 하나 가정하고 생성
// 추가적으로 더 필요하다면 수정 해서 사용  할 것
/*------------------------------*/

class RedisManager
{
public:
    static RedisManager& GetInstance()
    {
        static RedisManager instance;
        return instance;
    }

    RedisManager();
    ~RedisManager();
public:
    //command 관련
    template<typename ...Args>
    void RAsyncCommand(std::string_view format, Args&&... args);
    template<typename Callback, typename... Args>
    void RAsyncCommandCallback(Callback&& callback, std::string_view format, Args&&... args);

    void CommandNode_RunEventLoopOnce();

    //pubsubNode 관련
    void PubSubNode_RunEventLoopOnce();

private:
    shared_ptr<RedisConnection> _commandNode;
    shared_ptr<RedisPubSubConnection> _pubsubNode;

private:
    USE_LOCK;
    RedisManager(const RedisManager&) = delete;
    RedisManager& operator=(const RedisManager&) = delete;

};

template<typename ...Args>
inline void RedisManager::RAsyncCommand(std::string_view format, Args && ...args)
{
    if (!_commandNode->GetContext())
    {
        std::cerr << "Redis context is null!" << std::endl;
        return;
    }

    struct CommandWrapper
    {
        std::string format;
    };

    auto* cbWrapper = new CommandWrapper{ std::string(format) };

    redisAsyncCommand(_commandNode->GetContext() , [](redisAsyncContext* ctx, void* reply, void* privdata)
    {
        auto* wrapper = static_cast<CommandWrapper*>(privdata);
        if (wrapper)
        {
            RedisUtils::ReplyResponseHandler(reply, wrapper->format);
            delete wrapper;
        }
    }, cbWrapper, format.data(), std::forward<Args>(args)...);

}

template<typename Callback, typename ...Args>
inline void RedisManager::RAsyncCommandCallback(Callback&& callback, std::string_view format, Args && ...args)
{
    if (!_commandNode->GetContext())
    {
        std::cerr << "Redis context is null!" << std::endl;
        return;
    }

    struct CallbackWrapper
    {
        std::string format;
        std::function<void(redisReply*)> callback;
    };

    auto* cbWrapper = new CallbackWrapper{
        std::string(format),
        std::function<void(redisReply*)>(std::forward<Callback>(callback))
    };

    redisAsyncCommand(_commandNode->GetContext(), [](redisAsyncContext* ctx, void* reply, void* privdata)
    {
       auto* wrapper = static_cast<CallbackWrapper*>(privdata);
       if (wrapper)
       {
           wrapper->callback(static_cast<redisReply*>(reply));
           delete wrapper;
       }
    }, cbWrapper, format.data(), std::forward<Args>(args)...);
}
