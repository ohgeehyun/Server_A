#pragma once
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include "functional"


enum class ServiceType : uint8
{
    Server,
    Client
};
/*----------------------
        Service
----------------------*/
using SessionFactory = function<SessionRef(void)>;

class Service : public enable_shared_from_this<Service>
{
public:
    Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory,int32 maxSessionCount = 1);
    //address를 동적으로 추가하는 형식
    Service(ServiceType type,IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
    virtual ~Service();

    virtual bool Start() abstract;
    bool		 CanStart() { return _sessionFactory != nullptr; }

    virtual void CloseService();
    void		 SetSessionFactory(SessionFactory func) { _sessionFactory = func; }
    void		 Broadcast(SendBufferRef sendBuffer);
    SessionRef	 CreateSession();
    SessionRef   CreateSession(const NetAddress& address);
    void		 AddSession(SessionRef session);
    void		 ReleaseSession(SessionRef session);
    int32		 GetCurrentSessionCount() { return _sessionCount; }
    int32		 GetMaxSessionCount() { return _maxSessionCount; }
public:
    ServiceType	 GetServiceType() { return _type; }
    NetAddress   GetNetAddress() { return _netAddress; }
    IocpCoreRef& GetIocpCore() { return _iocpCore; }

protected:
    USE_LOCK;
    ServiceType		_type;
    NetAddress		_netAddress = {};
    IocpCoreRef		_iocpCore;

    Set<SessionRef>	_sessions;
    int32			_sessionCount = 0;
    int32			_maxSessionCount = 0;
    SessionFactory	_sessionFactory;//세션을 생성하는 함수를 저장할 함수포인터
};

/*----------------------
      ClientService
-----------------------*/
class ClientService : public Service
{
public:
    ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
    ClientService(IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
    
    void Add_ClientInfo(NetAddress Address);
    void AddClientInfoAndStart(NetAddress Address);

    virtual ~ClientService() {};
    virtual bool Start() override;

private:
    //주소 정보를 나중에 받기 위한 Service를 구분하기 위한 flag
    bool _runtimeNetAddress = false;
    vector<NetAddress> _networkAddressList;
};

/*----------------------
      ServerService
-----------------------*/
class ServerService : public Service
{
public:
    ServerService(NetAddress Address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
    virtual		~ServerService() {};

    virtual bool Start() override;
    virtual void CloseService() override;
private:
    ListenerRef  _listener = nullptr;
};