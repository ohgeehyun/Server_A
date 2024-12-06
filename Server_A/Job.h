#pragma once
#include <tuple>

class IJob {
public:
    virtual void Execute() = 0;
    virtual ~IJob() = default;
};

template <typename R,typename ...Args>
class Job : public IJob {
public:
    explicit Job(std::function<R(Args...)> action, std::tuple<Args...> args): _action(std::move(action)), _args(std::move(args)) {} 
    
    /*apply의경우 c++ 17이후 추가되었다. 현재 c++ 14 기준으로 작성되어 사용하지않음. }*/
    R Execute() override { Invoke(std::make_index_sequence<sizeof...(Args)>{}); }
private:
    template <std::size_t... Indexes>
    void Invoke(std::index_sequence<Indexes...>) {
        _action(std::get<Indexes>(_args)...);
    }

    std::function<R(Args...)> _action;
    std::tuple<Args...> _args;
};

