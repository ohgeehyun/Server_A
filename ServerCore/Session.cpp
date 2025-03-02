#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"


/*---------------------
        Session
----------------------*/

Session::Session() :_recvBuffer(BUFFER_SIZE)
{
    _socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
    SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
    if (IsConnected() == false)
        return;

    bool registerSend = false;

    {
        WRITE_LOCK;
        _sendQueue.push(sendBuffer);

        if (_sendRegistered.exchange(true) == false)
            registerSend = true;
    }
    if (registerSend)
        RegisterSend();

}

bool Session::Connect()
{
    //분산 서버의 경우 서버랑 서버끼리 통신
    return RegisterConnect();
}

void Session::DisConnect(const WCHAR* cause)
{
    if (_connected.exchange(false) == false)
        return;

    //TEMP
    wcout << "DISconnect : " << cause << endl;

    RegisterDisConnect();
}

HANDLE Session::GetHandle()
{
    return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
    switch (iocpEvent->eventType)
    {
    case EventType::Connect:
        ProcessConnect();
        break;
    case EventType::DisConnect:
        processDisConnect();
        break;
    case EventType::Recv:
        ProcessRecv(numOfByte);
        break;
    case EventType::Send:
        ProcessSend(numOfByte);
        break;
    default:
        break;
    }
}

bool Session::RegisterConnect()
{
    if (IsConnected())
        return false;

    if (GetService()->GetServiceType() != ServiceType::Client)
        return false;

    if (SocketUtils::BindAnyAddress(_socket, 0/*남는 포트*/))

        _connectEvent.Init();
    _connectEvent.owner = shared_from_this(); //ADD_REF

    DWORD numOfBytes = 0;
    SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();

    if (false == SocketUtils::connectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockaddr), nullptr, 0, &numOfBytes, &_connectEvent))
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            _connectEvent.owner = nullptr;//RELESE REF
            return false;
        }
    }
}

bool Session::RegisterDisConnect()
{
    _disconnectEvent.Init();
    _disconnectEvent.owner = shared_from_this();

    if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            _disconnectEvent.owner = nullptr; //RELEASE_REF
            return true;
        }
    }
    return true;
}

void Session::RegisterRecv()
{
    if (IsConnected() == false)
        return;

    _recvEvent.Init();
    _recvEvent.owner = shared_from_this();//ADD_REF

    WSABUF wsaBuf;
    wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
    wsaBuf.len = _recvBuffer.FreeSize();

    DWORD numOfBytes = 0;
    DWORD flags = 0;
    if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &_recvEvent, nullptr))
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            HandleError(errorCode);
            _recvEvent.owner = nullptr; //RELEASE_REF
        }
    }
}

void Session::RegisterSend()
{
    if (IsConnected() == false)
        return;

    _sendEvent.Init(); //overlapped초기화
    _sendEvent.owner = shared_from_this(); //ADD_REF

    //보낼 데이터를 sendEvent에 등록
    {
        WRITE_LOCK;

        int32 writeSize = 0;
        while (_sendQueue.empty() == false)
        {
            SendBufferRef sendBuffer = _sendQueue.front();

            writeSize += sendBuffer->WriteSize();

            _sendQueue.pop();
            _sendEvent.sendbuffers.push_back(sendBuffer);
        }
    }

    Vector<WSABUF> wsaBufs;
    wsaBufs.reserve(_sendEvent.sendbuffers.size());

    for (SendBufferRef sendBuffer : _sendEvent.sendbuffers)
    {
        WSABUF wsaBuf;
        wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
        wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
        wsaBufs.push_back(wsaBuf);
    }

    DWORD numOfBytes = 0;
    if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
    {
        int32 errorCode = ::WSAGetLastError();
        if (errorCode != WSAGetLastError())
        {
            HandleError(errorCode);
            _sendEvent.owner = nullptr; //RELESE_REF
            _sendEvent.sendbuffers.clear();
            _sendRegistered.store(false);
        }
    }
}

void Session::ProcessConnect()
{
    _connectEvent.owner = nullptr; //RELEASE_REF

    _connected.store(true);

    //세션 등록
    GetService()->AddSession(GetSessionRef());

    //컨텐츠 코드에서 재정의
    OnConnected();

    //수신등록
    RegisterRecv();
}

void Session::processDisConnect()
{
    _disconnectEvent.owner = nullptr; //RELEASE_REF

    OnDisConnected(); //컨텐츠 코드에서 재정의

    GetService()->ReleaseSession(GetSessionRef());
}

void Session::ProcessRecv(int32 numOfBytes)
{
    _recvEvent.owner = nullptr; //RELEASE_REF
    if (numOfBytes == 0)
    {
        DisConnect(L"Recv 0");
        return;
    }

    if (_recvBuffer.OnWrite(numOfBytes) == false)
    {
        DisConnect(L"OnWrite Overflow");
        return;
    }

    int32 dataSize = _recvBuffer.DataSize();

    int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
    if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
    {
        DisConnect(L"OnRead Overflow");
        return;
    }
    //read write커서 초기화
    _recvBuffer.Clean();
    RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
    _sendEvent.owner = nullptr;
    _sendEvent.sendbuffers.clear();

    if (numOfBytes == 0)
    {
        DisConnect(L"SEND 0");
        return;
    }

    OnSend(numOfBytes);

    WRITE_LOCK;
    if (_sendQueue.empty())
        _sendRegistered.store(false);
    else
        RegisterSend();
}

void Session::HandleError(int32 errorCode)
{
    switch (errorCode)
    {
    case WSAECONNRESET:
    case WSAECONNABORTED:
        DisConnect(L"HandleError");
        break;
    default:
        // TODO : log
        cout << "Handle Error : " << errorCode << endl;
        break;
    }
}

PacketSession::PacketSession()
{
}

PacketSession::~PacketSession()
{
}

int32 PacketSession::OnRecv(BYTE* buffer, int32 len)
{
    uint16 processLen = 0;

    while (true)
    {
        uint16 dataSize = len - processLen;
        //최소한 헤더는 파싱 
        if (dataSize < sizeof(RecvPacketHeader))
            break;

        RecvPacketHeader header = *(reinterpret_cast<RecvPacketHeader*>(&buffer[processLen]));

        if (dataSize < header.size)
            break;
        //패킷 조립 성공
        OnRecvPacket(&buffer[processLen], header.size);

        processLen += header.size;
    }

    return processLen;
}