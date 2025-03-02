#include "pch.h"
#include "MysqlCommand.h"


std::shared_ptr<sql::Connection> MysqlCommand::Connect(const std::string& host, const std::string& port, const std::string& username, const std::string& password, const std::string& dbname)
{
    try {
     
        std::string dns = "tcp://" + host + ":" + port;
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::shared_ptr<sql::Connection> con(driver->connect(dns, username, password));

        con->setSchema(dbname); // 사용할 데이터베이스 선택
        std::cout << "Connected to the database: " << dbname << std::endl;
        return con;
    }
    catch (const sql::SQLException& e) {
        MysqlErrorHandler(e);
        return nullptr;
    }
}

bool MysqlCommand::DisConnect(std::shared_ptr<sql::Connection>& con)
{
    try {
        con->close();
        std::cout << "MySQL connection closed successfully." << std::endl;
        return true;
    }
    catch (const sql::SQLException& e) {
        MysqlErrorHandler(e);
        return false;
    }
}



void MysqlCommand::MysqlErrorHandler(const sql::SQLException& e)
{
    std::cerr << "SQLException occurred: " << e.what() << std::endl;
    std::cerr << "Error code: " << e.getErrorCode() << std::endl;
    std::cerr << "SQLState: " << e.getSQLState() << std::endl;
}

