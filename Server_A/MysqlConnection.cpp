#include "pch.h"
#include "MysqlConnection.h"
#include <boost/asio.hpp>
#include "MysqlConnectionPool.h"
/*------------------------
     MysqlConnection
-------------------------*/

MysqlConnection::MysqlConnection(std::shared_ptr<boost::asio::io_context> io_context) : _resolver(*io_context), _socket(*io_context) ,_io_context(io_context)
{
   
}

MysqlConnection::~MysqlConnection()
{
}

void MysqlConnection::Init()
{
    //삭제 전 shared_ptr로 관리 되던 객체의 참조를 끊어 메모리 해제
    _connector = nullptr;
    _io_context = nullptr;
}

void MysqlConnection::AsyncConnect(const std::string& host, const std::string& port, const std::string& username, const std::string& password, const std::string& dbname)
{
    _resolver.async_resolve(host, port,
        [this, host, port, username, password, dbname](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {

        if (ec) {
            AsioErrorHandler(ec);
            return;
        }

        // 호스트와 포트가 해결되면 비동기적으로 연결 시작
        boost::asio::async_connect(_socket, results,
            [this, host, port, username, password, dbname](const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint&) {

            if (ec) {
                AsioErrorHandler(ec);
                return;
            }

            // 연결 성공 시, MySQL 연결을 설정
            _connector = MysqlCommand::Connect(host, port, username, password, dbname);
        });
    });

    // 비동기 작업들이 완료될 때까지 실행 (한 번만 호출)
    _io_context->run();
}

void MysqlConnection::AsyncDisConnect()
{
    if (_connector == nullptr)
        return;
    
    IO_Context_Restart();

    boost::asio::post(_io_context->get_executor(), [this](){
       
        if (!MysqlCommand::DisConnect(_connector))
            return;
        GDBConnectionPool->Sub_ConnectCount();
       _socket.close();
    });

}

void MysqlConnection::IO_Context_Restart()
{
    _io_context->restart();
}

void MysqlConnection::IO_Context_Run()
{
    _io_context->run();
}


void MysqlConnection::AsioErrorHandler(const boost::system::error_code& ec)
{
    if (ec.value() == 0)
        return;

    cout << "asio 비동기 연결 실패 에러 번호:  " << ec.value() << "\n";
    cout << "asio 비동기 연결 실패 메세지 :  " << ec.message() << "\n";
}