#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
    class InsertGold : public DBBind<3,0>
    {
    public:
    	InsertGold(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertGold(?,?,?)}") { }
    	void In_Gold(int32& v) { BindParam(0, v); };
    	void In_Gold(int32&& v) { _gold = std::move(v); BindParam(0, _gold); };
    	void In_Name(nvarchar& v) { BindParam(1, v); };
    	void In_Name(nvarchar&& v) { _name = std::move(v); BindParam(1, _name); };
    	void In_CreateDate(TIMESTAMP_STRUCT& v) { BindParam(2, v); };
    	void In_CreateDate(TIMESTAMP_STRUCT&& v) { _createDate = std::move(v); BindParam(2, _createDate); };

    private:
    	int32 _gold = {};
    	TIMESTAMP_STRUCT _createDate = {};
    };

    class GetGold : public DBBind<1,0>
    {
    public:
    	GetGold(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spGetGold(?)}") { }
    	void In_Gold(int32& v) { BindParam(0, v); };
    	void In_Gold(int32&& v) { _gold = std::move(v); BindParam(0, _gold); };

    private:
    	int32 _gold = {};
    };


     
};