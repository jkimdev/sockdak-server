//
//  database.hpp
//  sockdak-server
//
//  Created by 김재민 on 1/28/24.
//

#ifndef database_hpp
#define database_hpp

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/mysql/handshake_params.hpp>
#include <boost/mysql/row_view.hpp>
#include <boost/mysql/tcp_ssl.hpp>
#include <boost/mysql/throw_on_error.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <exception>
#include <iostream>

using boost::mysql::error_code;

class Database {
public:
    Database(const std::string& username, const std::string& password, const std::string& db, const std::string& hostname);
    void MainImpl(const std::string& username, const std::string& password, const std::string& db);
    void run();
private:
    boost::asio::io_context ctx_;
    boost::asio::ssl::context ssl_ctx_;
    boost::mysql::tcp_ssl_connection conn_;
    boost::mysql::handshake_params params_;
    boost::asio::ip::tcp::resolver resolver_;
    std::string hostname_;
    
    boost::asio::awaitable<void> coro_main(boost::mysql::tcp_ssl_connection& conn, boost::asio::ip::tcp::resolver& resolver, const boost::mysql::handshake_params& params, const char* hostname);
    void PrintResult(boost::mysql::results result);
    void CloseHandler(const boost::system::error_code& ec);
};

#endif /* database_hpp */
