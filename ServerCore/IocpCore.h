#pragma once

/*------------------
     IocpObject
--------------------*/
//CreateIoCompletionPort 의 키 값과 GetQueueCompletionStatus할 때 overlapped를 상속받을 구조체
//그중 key값을 만들어줄 class
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
    virtual ~IocpObject();

    virtual HANDLE GetHandle() abstract;
    virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) abstract;
};
/*-------------------
       IocpCore
--------------------*/
class IocpCore
{
public:
    IocpCore();
    ~IocpCore();

    HANDLE GetHandle() { return _iocpHandle; }

    bool Register(IocpObjectRef iocpObject);
    bool Dispatch(uint32 timeoutMs = INFINITE); //상황에 따른 GetQueueCompletionStatus 호출

private:
    HANDLE _iocpHandle;
};