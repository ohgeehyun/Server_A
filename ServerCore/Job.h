#pragma once
#include <functional>

/*---------------------
           Job
-----------------------*/

using CallbackType = std::function<void()>;

class Job
{
public:
    Job(CallbackType&& callback) : _callback(std::move(callback))
    {

    }

    template<typename T, typename Ret, typename...Args>
    Job(shared_ptr<T> owner, Ret(T::*memFunc)(Args...), Args&&...args)
    {
        // 람다 캡처에서 shared_ptr와 args를 이동
        _callback = [owner = std::move(owner), memFunc, args = std::make_tuple(std::forward<Args>(args)...)]() mutable 
        {
            std::apply([owner, memFunc](auto&&... unpackedArgs) {
                (owner.get()->*memFunc)(std::forward<decltype(unpackedArgs)>(unpackedArgs)...);
            }, args);
        // std::apply를 사용하여 튜플을 풀어 전달
        };
    }

    void Execute()
    {
        _callback();
    }

private:
    CallbackType _callback;
};