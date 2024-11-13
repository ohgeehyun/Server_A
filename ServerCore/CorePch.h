#pragma once
//PreCompiled Header

#include "Types.h"
#include "CoreGlobal.h"
#include "CoreTLS.h"
#include "CoreMacro.h"
#include "Container.h"
#include "ThreadManager.h"
#include "SendBuffer.h"

#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#include <windows.h>
#include <iostream>
#include <thread>
#include <memory>

#include "Lock.h"
#include "ObjectPool.h"
#include "Memory.h"
#include "Service.h"
#include "Session.h"



using namespace std;