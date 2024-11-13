#pragma once
#include "IocpCore.h"
#include "NetAddress.h"
/*------------------
      Listener
------------------*/
class AcceptEvent;
class ServerService;
class Listener : public IocpObject
{
public:
    Listener() = default;
    ~Listener();
public:
    bool StartAccept(ServerServiceRef service); // TODO : 서비스에 따른 listensocket의 start
    void CloseSocket();
public:
    /*IocpObject 가상함수 구현*/
    virtual HANDLE GetHandle() override;
    virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) override;
public:
    void RegisterAccept(AcceptEvent* acceptEvent);
    void ProcessAccept(AcceptEvent* acceptEvent);
protected:
    SOCKET _socket = INVALID_SOCKET;
    Vector<AcceptEvent*> _acceptEvents;
    ServerServiceRef _service;

};