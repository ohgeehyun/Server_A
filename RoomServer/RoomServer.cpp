#pragma once
#include "pch.h"
#include "UserServerSession.h"
#include "SessionManager.h"



int main()
{
    //console output 인코딩을 utf8사용 그렇지 않으면 기본적으로 console창의 인코딩은 운영체제 설정에 따라가는 것 같음. 기본적으로 window에서는 utf-8이 꺼저있더라..
    SetConsoleOutputCP(CP_UTF8); 

    UserServerSessionManager = new SessionManager();

    ServerServiceRef service = Make_Shared<ServerService>(
        NetAddress(L"220.81.12.171",5253),
        make_shared<IocpCore>(),
        make_shared<UserServerSession>,
        10);


    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; i++)
    {
        GThreadManager->Launch([&service]() {
            while (true)
            {
                service->GetIocpCore()->Dispatch(10);
            }
        });
    }

    GThreadManager->Join();
}

