#pragma once
#include "MysqlConnection.h"

/*------------------------
     MysqlConnectionPool
-------------------------*/
class MysqlConnectionPool
{
public:
    MysqlConnectionPool(int32 ConnCount);
    ~MysqlConnectionPool();

    template<typename ...Params>
    void AsyncInsertRegister(std::string& query,Params...parms);
    template<typename ...Params>
    void AsyncUpdateRegister(std::string& query, Params...parms);
    template<typename ...Params>
    void AsyncSelectRegister(std::string& query,Params...parms);

    void IoContextStart();
    void Sub_ConnectCount();

    void DisConnect();
    void Clear();

private:
    bool Connect(int32 ConnCount);

private:
    USE_LOCK;
    Vector<MysqlConnectionRef> _Pool;
    Atomic<int32> _connectCount = 0;
    Atomic<int32> _workerCount = 0;
};


template<typename ...Params>
void MysqlConnectionPool::AsyncInsertRegister(std::string& query, Params...parms)
{
    if (_Pool[_workerCount]->GetConnector() == nullptr)
        return;

    WRITE_LOCK
    {
        //여기서 workerCount떠에 따라 해당 번호의 소켓이 DB처리
        if (_Pool[_workerCount]->GetContext()->stopped() == true)
            _Pool[_workerCount]->IO_Context_Restart();

          // TODO : 비동기로 이벤트 등록
          _Pool[_workerCount]->AsyncInsertRecord(query, parms...);
          
          //_Pool[_workerCount]->IO_Context_Run();
    }
    _workerCount.fetch_add(1);
}

template<typename ...Params>
void MysqlConnectionPool::AsyncUpdateRegister(std::string& query, Params...parms)
{
    if (_Pool[_workerCount]->GetConnector() == nullptr)
        return;

    WRITE_LOCK
    {
        //여기서 workerCount떠에 따라 해당 번호의 소켓이 DB처리
        if (_Pool[_workerCount]->GetContext()->stopped() == true)
            _Pool[_workerCount]->IO_Context_Restart();

          // TODO : 비동기로 이벤트 등록
          _Pool[_workerCount]->AsyncUpdateRecord(query, parms...);

          //_Pool[_workerCount]->IO_Context_Run();
    }
    _workerCount.fetch_add(1);
}

template<typename ...Params>
void MysqlConnectionPool::AsyncSelectRegister(std::string& query, Params...parms)
{
    if (_Pool[_workerCount]->GetConnector() == nullptr)
        return;

    WRITE_LOCK
    {
        //여기서 workerCount떠에 따라 해당 번호의 소켓이 DB처리
        if (_Pool[_workerCount]->GetContext()->stopped() == true)
            _Pool[_workerCount]->IO_Context_Restart();

         // TODO : 비동기로 이벤트 등록
         _Pool[_workerCount]->AsyncSelectRecord(query, parms...);

         //_Pool[_workerCount]->IO_Context_Run();
    }
    _workerCount.fetch_add(1);
}