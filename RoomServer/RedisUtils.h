#pragma once
class RedisUtils
{
public:
    template<typename ...Args>
    static void RAsyncCommand(redisAsyncContext* context, const char* format, Args... args);
    template<typename Callback, typename... Args>
    static void RAsyncCommand(redisAsyncContext* context, Callback&& callback, const char* format, Args&&... args);

    static void replyResponseHandler(void* reply,const char* log);
    static void testGetvalue(void* reply);

private:

};

template<typename ...Args>
inline void RedisUtils::RAsyncCommand(redisAsyncContext* context, const char* format, Args ...args)
{
    if (!context)
    {
        std::cerr << "Redis context is null!" << std::endl;
        return;
    }

    struct commandWrapper
    {
        const char* commandFormat;
    };

    auto* cbWrapper = new commandWrapper{ format };

    redisAsyncCommand(context, [](redisAsyncContext* ctx, void* reply, void* privdata)
    {
        auto* wrapper = static_cast<commandWrapper*>(privdata);
        if (wrapper)
        {

            RedisUtils::replyResponseHandler(reply, wrapper->commandFormat);

            delete wrapper;  // 메모리 해제
        }
    }, cbWrapper, format, args...);
}


template<typename Callback, typename... Args>
void RAsyncCommand(redisAsyncContext* context, Callback&& callback, const char* format, Args&&... args)
{
    struct CallbackWrapper
    {
        const char* format;
        std::function<void(redisReply*)> callback;
    };

    auto* cbWrapper = new CallbackWrapper{
         format,
         std::function<void(redisReply*)>(std::forward<Callback>(callback))
    };

    redisAsyncCommand(context, [](redisAsyncContext* ctx, void* reply, void* privdata)
    {
        auto* wrapper = static_cast<CallbackWrapper*>(privdata);
        wrapper->callback(static_cast<redisReply*>(reply));
        delete wrapper;

    }, cbWrapper, format, std::forward<Args>(args)...);
}