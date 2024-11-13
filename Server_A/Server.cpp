#pragma once
#include "pch.h"
#include "GameSession.h"
#include "ClientPacketHandler.h"
#include "GameSessionManager.h"
#include <windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    ClientPacketHandler::Init();
    SessionManager = new GameSessionManager();

    ServerServiceRef service = Make_Shared<ServerService>(
        NetAddress(L"127.0.0.1", 5252),
        make_shared<IocpCore>(),
        make_shared<GameSession>,
        100);

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
    {
        GThreadManager->Launch([=]() {
            while (true)
            {
                service->GetIocpCore()->Dispatch();
            }
        });
    }
    GThreadManager->Join();

}