#pragma once

class RedisUtils
{

public:
    template<typename ...Args>
    static void RAsyncCommand(redisAsyncContext* context, std::string_view format, Args&&... args);

    template<typename Callback, typename... Args>
    static void RAsyncCommandCallback(redisAsyncContext* context, Callback&& callback, std::string_view format, Args&&... args);

    static void ReplyResponseHandler(void* reply, std::string_view log);
    static void TestGetValue(void* reply);

};

template<typename ...Args>
inline void RedisUtils::RAsyncCommand(redisAsyncContext* context, std::string_view format, Args&& ...args)
{
    if (!context)
    {
        std::cerr << "Redis context is null!" << std::endl;
        return;
    }

    struct CommandWrapper
    {
        std::string format;
    };

    auto* cbWrapper = new CommandWrapper{ std::string(format) };

    redisAsyncCommand(context, [](redisAsyncContext* ctx, void* reply, void* privdata)
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
inline void RedisUtils::RAsyncCommandCallback(redisAsyncContext* context, Callback&& callback, std::string_view format, Args&& ...args)
{
    if (!context)
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

    redisAsyncCommand(context, [](redisAsyncContext* ctx, void* reply, void* privdata)
    {
        auto* wrapper = static_cast<CallbackWrapper*>(privdata);
        if (wrapper)
        {
            wrapper->callback(static_cast<redisReply*>(reply));
            delete wrapper;
        }
    }, cbWrapper, format.data(), std::forward<Args>(args)...);
}
