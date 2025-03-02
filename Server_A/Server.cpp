#pragma once
#include "pch.h"
#include "GameSession.h"
#include "RoomManager.h"
#include "ClientPacketHandler.h"
#include "GameSessionManager.h"
#include "ConfigManager.h"
#include "DataManager.h"
#include "Room.h"
#include "MysqlConnectionPool.h"
#include "RedisConnection.h"

enum
{
    WORKER_TICK = 64,
};

void  DoWorkerJob(ServerServiceRef& service)
{
    while (true)
    {
        //LEndTickCount = ::GetTickCount64() + WORKER_TICK;

        //네트워크 입출력 처리 -> 인게임 로직까지 호출하는 상황이었음(패킷 핸들러에 의해)
        service->GetIocpCore()->Dispatch(10);

        ThreadManager::DistributeReservedJobs();
        //글로벌 큐
        ThreadManager::DoGlobalQueueWork();
    }
}

int main()
{
    //CALL $(SolutionDir)Common\Protobuf\bin\GenPackets.bat 테스트중에 빌드 전이벤트 잠시 종료

    SetConsoleOutputCP(CP_UTF8);

    ConfigManager::GetInstance().LoadConfig();
  
    //서버 기동 전 플레이어 스탯정보와 skill 읽어오기
    auto  stat = DataManager::GetInstance().GetStatDict();
    auto  skill = DataManager::GetInstance().GetSkillDict();
    
    ClientPacketHandler::Init();
    SessionManager = new GameSessionManager();

    ServerServiceRef service = Make_Shared<ServerService>(
        NetAddress(L"220.81.12.171", 5252),
        make_shared<IocpCore>(),
        make_shared<GameSession>,
        100);

    GDBConnectionPool = new MysqlConnectionPool(3);
    GRedisConnection = new RedisConnection();


    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
    {
        GThreadManager->Launch([&service]() {
            while (true)
            {
                DoWorkerJob(service);
            }
        });
    }

    for (int32 i = 0; i < 2; i++)
    {
        GThreadManager->Launch([]() {
            while (true)
            {
               GDBConnectionPool->IoContextStart();
            }
        });
    }

    GThreadManager->Launch([]() {
        while (true)
            GRedisConnection->RunEventLoop();
    });

    GThreadManager->Launch([]() {
        while (true)
        {
            //RoomRef room = RoomManager::GetInstance().Find(1);
            //if (room == nullptr)
            //    continue;
            //room->DoTimer(100,std::bind(&Room::Update,room));
            //room->DoTimer(100, &Room::Update);
            //room->Update();
            RoomManager::GetInstance().DoRoomUpdate();
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    });

    GThreadManager->Join();
}