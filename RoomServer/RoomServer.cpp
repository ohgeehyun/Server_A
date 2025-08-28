#pragma once
#include "pch.h"
#include "UserServerSession.h"
#include "SessionManager.h"
#include "RoomPacketHandler.h"
#include "RedisManager.h"
#include "RoomManager.h"
#include "ConfigManager.h"

void  DoWorkerJob(ServerServiceRef& service)
{
   //LEndTickCount = ::GetTickCount64() + WORKER_TICK;

   //네트워크 입출력 처리 -> 인게임 로직까지 호출하는 상황이었음(패킷 핸들러에 의해)
   service->GetIocpCore()->Dispatch(10);

   ThreadManager::DistributeReservedJobs();
   //글로벌 큐
   ThreadManager::DoGlobalQueueWork();
}
void DoRedisWorkJob()
{
    RedisManager::GetInstance().PubSubNode_RunEventLoopOnce();
    RedisManager::GetInstance().CommandNode_RunEventLoopOnce();
}

int main()
{
    //console output 인코딩을 utf8사용 그렇지 않으면 기본적으로 console창의 인코딩은 운영체제 설정에 따라가는 것 같음. 기본적으로 window에서는 utf-8이 꺼저있더라..
    SetConsoleOutputCP(CP_UTF8); 

    ConfigManager::GetInstance().LoadConfig();

    RoomPacketHandler::Init();

    UserServerSessionManager = new SessionManager();
    GRoomManager = Make_Shared<RoomManager>();

    ServerServiceRef service = Make_Shared<ServerService>(
        NetAddress(L"125.137.11.149",5253),
        make_shared<IocpCore>(),
        make_shared<UserServerSession>,
        10);

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
    {
        GThreadManager->Launch([&service]() {
            while (true)
                DoWorkerJob(service);
        });
    }

    GThreadManager->Launch([]() {
        while (true)
            DoRedisWorkJob();
    });

    GThreadManager->Launch([]() {
        while (true)
        {
            GRoomManager->DoRoomUpdate();
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    });

    GThreadManager->Join();
}

