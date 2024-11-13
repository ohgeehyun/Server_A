#pragma once

#ifdef _DEBUG
#pragma comment(lib,"ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib,"Protobuf\\Debug\\libprotobufd.lib")
#else
#pragma comment(lib,"ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib,"Protobuf\\Release\\libprotobuf.lib")
#endif

#include "CorePch.h"


using GameSessionRef = shared_ptr<class GameSession>;
//using PlayerRef = shared_ptr<class Player>;