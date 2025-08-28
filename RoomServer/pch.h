#pragma once

#ifdef _DEBUG
#pragma comment(lib,"ServerCore\\Debug\\ServerCore.lib")
#pragma comment(lib,"Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib,"hiredis\\Debug\\hiredisd.lib")
#pragma comment(lib,"hiredis\\Debug\\hiredis.lib")
#pragma comment(lib,"mysql\\Debug\\vs14\\mysqlcppconn.lib")
#pragma comment(lib,"OpenSSL\\MT\\libssl.lib")
#pragma comment(lib,"OpenSSL\\MT\\libcrypto.lib")
#else
#pragma comment(lib,"ServerCore\\Release\\ServerCore.lib")
#pragma comment(lib,"Protobuf\\Release\\libprotobuf.lib")
#pragma comment(lib,"hiredis\\Release\\hiredis.lib")
#pragma comment(lib,"hiredis\\Release\\hiredisd.lib")
#pragma comment(lib,"mysql\\Release\\v14\\mysqlcppconn.lib")
#pragma comment(lib,"OpenSSL\\MT\\libssl.lib")
#pragma comment(lib,"OpenSSL\\MT\\libcrypto.lib")
#endif

#include "CorePch.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <hiredis/adapters/libevent.h>
#include <boost/asio.hpp>
#include <hiredis/hiredis.h>
#include <sw/redis++/redis++.h>
#include <hiredis/async.h>
#include <event2/event.h>

#include "GlobalObject.h"
#include "RedisConnection.h"
#include "RedisPubSubConnection.h"
#include "RedisManager.h"
#include "JsonUtils.h"

using namespace std;

using UserServerSessionRef = shared_ptr<class UserServerSession>;
using RoomRef = shared_ptr<class Room>;
using GameObjectRef = shared_ptr<class GameObject>;
using PlayerRef = shared_ptr<class Player>;
using MonsterRef = shared_ptr<class Monster>;
using ArrowRef = shared_ptr<class Arrow>;
using MagicSkillRef = shared_ptr<class MagicSkill>;
using ProjectTileRef = shared_ptr<class ProjectTile>;
using RedisConnectionRef = shared_ptr<class RedisConnection>;
using RedisPubSubConnectionRef = shared_ptr<class RedisPubSubConnection>;
using RoomManagerRef = shared_ptr<class RoomManager>;