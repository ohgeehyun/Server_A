#pragma once
#include <sql.h>
#include <sqlext.h>

/*-----------------------
      DBConnection
-------------------------*/

enum
{
    WVARCHAR_MAX = 4000,
    BINARY_MAX = 8000
};

class DBConnection
{
public:
    bool Connect(SQLHENV henv, const WCHAR* connectionString);
    void Clear();

    //실행
    bool Execute(const WCHAR* query);
    //실행 결과 
    bool Fetch();
    int32 GetRowCount();
    void Unbind();

public:
    bool BindParam(int32 ParamIndex, bool* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, float* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, double* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, int8* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, int16* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, int32* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, int64* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
    bool BindParam(int32 ParamIndex, const WCHAR* str, SQLLEN* index);
    bool BindParam(int32 ParamIndex, BYTE* bin, int32 size, SQLLEN* index);

    bool BindCol(int32 columnIndex, bool* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, float* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, double* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, int8* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, int16* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, int32* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, int64* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
    bool BindCol(int32 columnIndex, WCHAR* str, int32 size, SQLLEN* index);
    bool BindCol(int32 columnIndex, BYTE* bin, int32  size, SQLLEN* index);

private:
    bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
    bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);
    void HandleError(SQLRETURN ret);
private:
    //커넥션을 담당할 핸들
    SQLHDBC _connection = SQL_NULL_HANDLE;
    //상태를 관리 할 핸들
    SQLHSTMT _statement = SQL_NULL_HANDLE;
};

