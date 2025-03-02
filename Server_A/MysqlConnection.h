#pragma once
#include <asio.hpp>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include "MysqlCommand.h"
/*------------------------
     MysqlConnection
-------------------------*/

class MysqlConnection
{
public:
    MysqlConnection(std::shared_ptr<boost::asio::io_context> io_context);
    ~MysqlConnection();

    void Init();
    void AsyncConnect(const std::string& host, const std::string& port, const std::string& username, const std::string& password, const std::string& dbname);
    void AsyncDisConnect();
    
    template<typename ...Params>
    void AsyncInsertRecord(std::string& query, Params...parms);
    
    template<typename ...Params>
    void AsyncUpdateRecord(std::string& query, Params...parms);
  
    template<typename ...Params>
    void AsyncSelectRecord(std::string& query, Params...parms);
    
    std::shared_ptr<sql::Connection>& GetConnector() { return _connector; };
    std::shared_ptr<boost::asio::io_context>& GetContext() { return _io_context;
    }
    void IO_Context_Restart();
    void IO_Context_Run();

private:
    void AsioErrorHandler(const boost::system::error_code& ec);
private:

    std::shared_ptr<sql::Connection> _connector;
    std::shared_ptr<boost::asio::io_context> _io_context;
    boost::asio::ip::tcp::resolver _resolver;
    boost::asio::ip::tcp::socket _socket;
};

template<typename ...Params>
void MysqlConnection::AsyncInsertRecord(std::string& query, Params...params)
{
    if (_connector == nullptr)
        return;

    boost::asio::post(_io_context->get_executor(), [this, query, params = std::make_tuple(std::forward<Params>(params)...)]() {

        MysqlCommand dbBind;

        std::apply([=](auto&&... args) {
            dbBind.executeInsertQuery(_connector, query, std::forward<decltype(args)>(args)...);
        }, params);
    });
}

template<typename ...Params>
void MysqlConnection::AsyncUpdateRecord(std::string& query, Params...params)
{
    if (_connector == nullptr)
        return;

    boost::asio::post(_io_context->get_executor(), [this, query, params = std::make_tuple(std::forward<Params>(params)...)]() {

        MysqlCommand dbBind;

        std::apply([=](auto&&... args) {
            dbBind.executeUpdateQuery(_connector, query, std::forward<decltype(args)>(args)...);
        }, params);
    });
}

template<typename ...Params>
void MysqlConnection::AsyncSelectRecord(std::string& query,Params...params)
{
    if (_connector == nullptr)
        return;

    boost::asio::post(_io_context->get_executor(), [this, query,params = std::make_tuple(std::forward<Params>(params)...)](){

        MysqlCommand dbBind;
        sql::ResultSet* result = nullptr;
        std::apply([=,&result](auto&&... args) {
            dbBind.executeSelectQuery(_connector, query,result, std::forward<decltype(args)>(args)...);
        }, params);  

        while (result->next())
        {
            int32 id = result->getInt("id");
            int32 gold = result->getInt("gold");
            std::string name = result->getString("name");

            std::cout << "ID: " << id << ", gold: " << gold << ", name: " << name << '\n';
        };
    });
}