#pragma once
/*--------------------
      NetAddress
--------------------*/
class NetAddress
{
public:
    NetAddress() = default;
    NetAddress(SOCKADDR_IN sockAddr);
    NetAddress(wstring ip, uint16 port);

    const SOCKADDR_IN& GetSockAddr() const { return _sockAddr; };
    const wstring GetIpAdress() const;
    const uint16 GetPort() const { return ::ntohs(_sockAddr.sin_port); }

public:
    static IN_ADDR ip2Address(const WCHAR* ip);

private:
    SOCKADDR_IN _sockAddr = {};
};