#pragma once
#include "pch.h"
#include "GameSession.h"
#include "RoomManager.h"
#include "ClientPacketHandler.h"
#include "GameSessionManager.h"
#include "ConfigManager.h"
#include "DataManager.h"
#include "Room.h"


int main()
{
    SetConsoleOutputCP(CP_UTF8);

    ConfigManager::GetInstance().LoadConfig();
  
    auto  stat = DataManager::GetInstance().GetStatDict();
    auto  skill = DataManager::GetInstance().GetSkillDict();

    RoomManager& manager = RoomManager::GetInstance();
    manager.Add(1);

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

    GThreadManager->Launch([]() {
        while (true)
        {
            RoomRef room = RoomManager::GetInstance().Find(1);
            std::function<void()> job = [room]() {
                room->Update();
            };
            room->Push(job);
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    });
    GThreadManager->Join();
}