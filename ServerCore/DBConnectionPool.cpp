#include "pch.h"
#include "DBConnectionPool.h"

/*------------------------
     DBConnectionPool
-------------------------*/

DBConnectionPool::DBConnectionPool()
{
}

DBConnectionPool::~DBConnectionPool()
{
    Clear();
}

bool DBConnectionPool::Connect(int32 connectionCount, const WCHAR* connectionString)
{
    //서버가 실행될때 딱 한번만 DB랑 연결시도
    WRITE_LOCK;

    //데이터베이스에 대한 다양한 핸들을 할당하는 역할 새롭게 생성된 핸들을 마지막 인자에 반환해줌
    //핸들의 유형,새로  생성할 핸들의 상위 컨텍스트,생성된 핸들의 값을 저장할 메모리 공간 주소
    if(::SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&_environment) != SQL_SUCCESS)
        return false;

    //odbc 버전 설정
    if (SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
        return false;

    for (int32 i = 0; i < connectionCount; i++)
    {
        DBConnection* connection = xnew<DBConnection>();
        if (connection->Connect(_environment, connectionString) == false)
            return false;

        _connections.push_back(connection);
    }
    return true;
}

void DBConnectionPool::Clear()
{
    WRITE_LOCK;
    if (_environment != SQL_NULL_HANDLE)
    {
        ::SQLFreeHandle(SQL_HANDLE_ENV,_environment);
        _environment = SQL_NULL_HANDLE;
    }

    for (DBConnection* connection : _connections)
        xdelete(connection);

    _connections.clear();
}

DBConnection* DBConnectionPool::Pop()
{
    WRITE_LOCK;

    if (_connections.empty())
        return nullptr;

    DBConnection* connection = _connections.back();
    _connections.pop_back();
    return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
    WRITE_LOCK;
    _connections.push_back(connection);
}
