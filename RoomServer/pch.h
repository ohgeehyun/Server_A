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
#include <hiredis/hiredis.h>
#include <sw/redis++/redis++.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <boost/asio.hpp>

using namespace std;

using UserServerSessionRef = shared_ptr<class UserServerSession>;


