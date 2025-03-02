#pragma once
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <tuple>
/*----------------------------
        MysqlCommand
----------------------------*/
class  MysqlCommand
{
public:
    static  std::shared_ptr<sql::Connection> Connect(const std::string& host, const std::string& port, const std::string& username, const std::string& password, const std::string& dbname);
    static  bool DisConnect(std::shared_ptr<sql::Connection>& con);

    template<typename ...Params>
    static void executeInsertQuery(std::shared_ptr<sql::Connection> conn, const std::string query, Params ...params);
    
    template<typename ...Params>
    static void executeUpdateQuery(std::shared_ptr<sql::Connection> conn, const std::string& query, Params... params);
    
    template<typename ...Params>
    static void executeSelectQuery(std::shared_ptr<sql::Connection> conn, const std::string& query, sql::ResultSet*& results ,Params... params);
private:

    static void MysqlErrorHandler(const sql::SQLException& e);

    template<typename T>
    static void bindSingleParam(sql::PreparedStatement* pstmt, int32 index, T param);
};


template<typename ...Params>
void MysqlCommand::executeInsertQuery(std::shared_ptr<sql::Connection> conn, const std::string query, Params ...params)
{
    try
    {
        //PreparedStatement 생성
        sql::PreparedStatement* pstmt = conn->prepareStatement(query);

        // param binding
        auto bind_all = [&pstmt](auto&&... args) {
            int32 index = 1;
            ((bindSingleParam(pstmt, index++, args)), ...);
        };

        bind_all(params...);


        //쿼리 실행
        pstmt->executeUpdate();
        cout << "Insert successful" << '\n';

        delete pstmt;
    }
    catch (sql::SQLException& e)
    {
        MysqlCommand::MysqlErrorHandler(e);
    }
}

template<typename ...Params>
void MysqlCommand::executeUpdateQuery(std::shared_ptr<sql::Connection> conn, const std::string& query, Params... params)
{
    try
    {
        sql::PreparedStatement* pstmt = conn->prepareStatement(query);

        // param binding
        auto bind_all = [&pstmt](auto&&... args) {
            int32 index = 1;
            ((bindSingleParam(pstmt, index++, args)), ...);
        };

        bind_all(params...);

        // Execute update query
        int affectedRows = pstmt->executeUpdate();
        std::cout << "Update successful, affected rows: " << affectedRows << '\n';

        delete pstmt; 
    }
    catch (const sql::SQLException& e)
    {
        MysqlCommand::MysqlErrorHandler(e);
    }
}

template<typename ...Params>
void MysqlCommand::executeSelectQuery(std::shared_ptr<sql::Connection> conn, const std::string& query,sql::ResultSet*& results,Params... params)
{
    try
    {
        sql::PreparedStatement* pstmt = conn->prepareStatement(query);

        // param binding
        auto bind_all = [&pstmt](auto&&... args) {
            int32 index = 1;
            ((bindSingleParam(pstmt, index++, args)), ...);
        };

        bind_all(params...);

        // Execute query and fetch results
        results = pstmt->executeQuery();

        delete pstmt;     // Clean up    
    }
    catch (const sql::SQLException& e)
    {
        MysqlCommand::MysqlErrorHandler(e);
    }
}

template<typename T>
void MysqlCommand::bindSingleParam(sql::PreparedStatement* pstmt, int32 index, T param)
{
    // 파라미터 타입에 맞는 바인딩
    if constexpr (std::is_integral<T>::value) {
        pstmt->setInt(index, param);  // 정수형 처리
    }
    else if constexpr (std::is_floating_point<T>::value) {
        pstmt->setDouble(index, param);  // 실수형 처리
    }
    else if constexpr (std::is_same<T, std::string>::value||
                       std::is_same<T, const char*>::value ||
                       std::is_same<T, char*>::value) {
        pstmt->setString(index, param);  // 문자열 처리
    }
    else
    {
        static_assert(false, "Unsupported type for binding!");
    }
}

