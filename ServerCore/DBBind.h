#pragma once
#include "DBConnection.h"


//재귀적으로 받은 수 만큼 재귀적으로 비트를 채우는 템플릿
template<int32 C>
struct FullBits { enum {value = (1 << (C -1)) | FullBits<C-1>::value }; };

template<>
struct FullBits<1> { enum { value = 1 }; };

template<>
struct FullBits<0> { enum {value = 0 }; };
/*--------------------------
            DB Bind
----------------------------*/

template<int32 ParamCount , int32 ColumnCount>
class DBBind
{
public:
    DBBind(DBConnection& dbConnection, const WCHAR* query)
        :_dbConnection(dbConnection), _query(query)
    {
        ::memset(_paramIndex, 0, sizeof(_paramIndex));
        ::memset(_columnIndex, 0, sizeof(_columnIndex));
        _paramFlag = 0;
        _columnFlag = 0;
        _dbConnection.Unbind();
    }

    bool Validate()
    {
        //정상적으로 만들어 둔 크기 만큼 데이터를 채웠는지 검증 
        return _paramFlag == FullBits<ParamCount>::value && _columnFlag == FullBits<ColumnCount>::value;
    }

    bool Execute()
    {
        ASSERT_CRASH(Validate());
        return _dbConnection.Execute(_query);
    }

    bool Fetch()
    {
        return _dbConnection.Fetch();
    }
public:
    template<typename T>
    void BindParam(int32 idx, T& value)
    {
        _dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]);
        // or 비트연산 1LL의 경우 LL은 LongLong형
        // 상수 1을 표현하는 64비트 자료형에서 idx번째에 1을 켜주는 것
        _paramFlag |= (1LL << idx);
    }

    void BindParam(int32 idx, const  WCHAR* value)
    {
        _dbConnection.BindParam(idx + 1 , value, &_paramIndex[idx]);
        _paramFlag |= (1LL << idx);
    }

    template<typename T,int32 N>
    void BindParam(int32 idx,T(&value)[N])
    {
        if constexpr (std::is_same_v<T, WCHAR>)
        {
            // WCHAR에 대한 처리: UTF-16 문자열로 전달
            _dbConnection.BindParam(idx + 1, (const WCHAR*)value, &_paramIndex[idx]);
        }
        else
        {
            // 기본 처리: BYTE 배열로 전달
            _dbConnection.BindParam(idx + 1, (const BYTE*)value, sizeof(T) * N, &_paramIndex[idx]);
        }
        _paramFlag |= (1LL << idx);
    }

    template<typename T>
    void  BindParam(int32 idx, T*value, int32 N)
    {
        _dbConnection.BindParam(idx+1,(const BYTE*)value, sizeof(T)*N, &_paramIndex[idx]);
        _paramFlag |= (1LL << idx);
    }

    template<typename T>
    void BindCol(int32 idx, T& value)
    {
        _dbConnection.BindCol(idx + 1, &value, &_columnIndex[idx]);
        _columnFlag |= (1LL << idx);
    }
    template<int32 N>
    void BindCol(int32 idx, WCHAR(&value)[N])
    {
        _dbConnection.BindCol(idx + 1, value , N-1, &_columnIndex[idx]);
        _columnFlag |= (1LL << idx);
    }

    void BindCol(int32 idx, WCHAR* value, int32 len)
    {
        _dbConnection.BindCol(idx+1,value,len-1,&_columnIndex[idx]);
        _columnFlag |= (1LL << idx);
    }

    template<typename T, int32 N>
    void BindCol(int32 idx, T(&value)[N])
    {
        _dbConnection.BindCol(idx+1,value, size32(T)*N, &_columnIndex[idx]);
        _columnFlag |= (1LL << idx);
    }


protected:
    DBConnection& _dbConnection;
    const WCHAR* _query;
    SQLLEN _paramIndex[ParamCount > 0 ? ParamCount : 1];
    SQLLEN _columnIndex[ColumnCount > 0 ? ColumnCount : 1];
    uint64 _paramFlag;
    uint64 _columnFlag;
};

