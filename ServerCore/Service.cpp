#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"

/*--------------------
        Service
-----------------------*/
Service::Service(ServiceType type, NetAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
    :_type(type), _netAddress(address), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{

}

Service::Service(ServiceType type, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
    :_type(type), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{

}

Service::~Service()
{
}

void Service::CloseService()
{
    // TODO : 서비스 종료시 해야할  점이 있다면 정의
}

void Service::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK;
    for (const auto& session : _sessions)
    {
        session->Send(sendBuffer);
    }
}

SessionRef Service::CreateSession()
{
    SessionRef session = _sessionFactory();
    session->SetService(shared_from_this());

    if (_iocpCore->Register(session) == false)
        return nullptr;

    return session;
}

SessionRef Service::CreateSession(const NetAddress& address)
{
    SessionRef session = _sessionFactory();
    session->SetNetAddress(address);
    session->SetService(shared_from_this());
    if (_iocpCore->Register(session) == false)
        return nullptr;

    return session;
}

void Service::AddSession(SessionRef session)
{
    WRITE_LOCK;
    {
       _sessionCount++;
       _sessions.insert(session);
    }
}

void Service::ReleaseSession(SessionRef session)
{
    WRITE_LOCK;
    {
        //세션의 개수가 맞지 않음.
       ASSERT_CRASH(_sessions.erase(session) != 0);
       _sessionCount--;
    }
}

ClientService::ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
    :Service(ServiceType::Client, targetAddress, core, factory, maxSessionCount)
{

}

ClientService::ClientService(IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
    :Service(ServiceType::Client,core, factory, maxSessionCount)
{
    _runtimeNetAddress = true;
}

void ClientService::Add_ClientInfo(NetAddress address)
{
    //해당 함수는 runtimeNetAddress 가 true일때 runtime에 netaddress를 추가하여 Client Service를 사용할떄 사용 됩니다.
    ASSERT_CRASH(_runtimeNetAddress);

    _networkAddressList.push_back(address);
    _maxSessionCount++;
}

void ClientService::AddClientInfoAndStart(NetAddress address)
{
    ASSERT_CRASH(_runtimeNetAddress);

    _networkAddressList.push_back(address);
    _maxSessionCount++;

    Start();
}

bool ClientService::Start()
{
    if (CanStart() == false)
        return false;

    if (_runtimeNetAddress == false)
    {
        const int32 sessionCount = GetMaxSessionCount();
        for (int32 i = 0; i < sessionCount; i++)
        {
            SessionRef session = CreateSession();
            if (session->Connect() == false)
                return false;
        }
    }
    else
    {
        for (auto it = _networkAddressList.begin(); it != _networkAddressList.end(); )
        {
            SessionRef session = CreateSession(*it);
            if (session->Connect(*it) == false)
            {
                it = _networkAddressList.erase(it); // 삭제 후, 다음 요소 가리키는 iterator 반환
                return false;
            }
            else
            {
                it++;
            }
        }
    }

    return true;
}


ServerService::ServerService(NetAddress Address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
    :Service(ServiceType::Server, Address, core, factory, maxSessionCount)
{
    GNetAddress = new NetAddress(Address);
}

bool ServerService::Start()
{
    if (CanStart() == false)
        return false;
    _listener = Make_Shared<Listener>();
    if (_listener == nullptr)
        return false;

    ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
    if (_listener->StartAccept(service) == false)
        return false;

    return true;
}

void ServerService::CloseService()
{
    // TODO 
    Service::CloseService();
}