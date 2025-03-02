#include "pch.h"
#include "MysqlConnectionPool.h"
#include "DataManager.h"
/*------------------------
     MysqlConnectionPool
-------------------------*/


MysqlConnectionPool::MysqlConnectionPool(int32 ConnCount)
{
    Connect(ConnCount);
}

MysqlConnectionPool::~MysqlConnectionPool()
{
    Clear();
}

bool MysqlConnectionPool::Connect(int32 ConnCount)
{
    HashMap<string, ServerConfigData> dict = DataManager::GetInstance().GetServerConfigDict();

    for (int32 i = 0; i < ConnCount; i++)
    {
        auto io_context = Make_Shared<boost::asio::io_context>();
        MysqlConnectionRef Conn = Make_Shared<MysqlConnection>(io_context);
        Conn->AsyncConnect(dict["database"].mysqlData.host, dict["database"].mysqlData.port, dict["database"].mysqlData.user, dict["database"].mysqlData.pwd, dict["database"].mysqlData.dbname);
        _Pool.push_back(Conn);
        _connectCount.fetch_add(1);
    }

    return true;
}

void MysqlConnectionPool::IoContextStart()
{
    if (_Pool.size() <= 0)
        return;

    for (int32 i = 0; i < _connectCount; i++)
    {
        if (_Pool[i]->GetContext()->stopped() == false)
            continue;
        _Pool[i]->IO_Context_Restart();
        _Pool[i]->IO_Context_Run();
    }
}

void MysqlConnectionPool::Sub_ConnectCount()
{
    _connectCount.fetch_sub(1);
}

void MysqlConnectionPool::DisConnect()
{
    for (int i = 0; i < _Pool.size(); i++)
    {
        _Pool[i]->AsyncDisConnect();
        Sub_ConnectCount();
    }
}

void MysqlConnectionPool::Clear()
{
    for (int i = 0; i < _Pool.size(); i++)
    {
        _Pool[i]->Init();
    }
    _Pool.clear();
}
