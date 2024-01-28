//
//  database.cpp
//  sockdak-server
//
//  Created by 김재민 on 1/28/24.
//

#include "database.hpp"

using boost::mysql::error_code;


Database::Database(const std::string& username, const std::string& password, const std::string& db, const std::string& hostname):
ctx_(),
ssl_ctx_(boost::asio::ssl::context::tls_client),
conn_(ctx_, ssl_ctx_), resolver_(ctx_.get_executor()),
params_(username, password, db),
hostname_(hostname) {}


boost::asio::awaitable<void> Database::coro_main(boost::mysql::tcp_ssl_connection &conn, boost::asio::ip::tcp::resolver &resolver, const boost::mysql::handshake_params &params, const char *hostname) {
    constexpr auto tuple_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);
    error_code ec;
    boost::mysql::diagnostics diag;
    
    auto endpoints = co_await resolver_.async_resolve(
                                                      hostname_,
                                                      boost::mysql::default_port_string,
                                                      boost::asio::use_awaitable
                                                      );
    
    std::tie(ec) = co_await conn_.async_connect(*endpoints.begin(), params_, diag, tuple_awaitable);
    boost::mysql::throw_on_error(ec, diag);
    
    boost::mysql::statement stmt;
    std::tie(ec, stmt) = co_await conn_.async_prepare_statement(
                                                                "SELECT 'Hello world!",
                                                                diag,
                                                                tuple_awaitable
                                                                );
    boost::mysql::throw_on_error(ec, diag);
    
    boost::mysql::results result;
    std::tie(ec) = co_await conn_.async_execute(stmt.bind(), result, diag, tuple_awaitable);
    boost::mysql::throw_on_error(ec, diag);
    
    
    Database::PrintResult(result);
    
    conn_.async_close(std::bind(&Database::CloseHandler, this, std::placeholders::_1));
}

void Database::CloseHandler(const boost::system::error_code &ec) {
    if (ec) {
        std::cout << "failed closing connection.." << std::endl;
    }
}

void Database::PrintResult(boost::mysql::results result) {
    std::cout << result.rows().at(0).at(0) << std::endl;
}

void Database::MainImpl(const std::string& username, const std::string& password, const std::string& db) {
    
    const char* hostname = "localhost";
        
    boost::mysql::tcp_ssl_connection conn(ctx_, ssl_ctx_);
    
    boost::mysql::handshake_params params(username, password, db);
    
    boost::asio::ip::tcp::resolver resolver(ctx_.get_executor());
    
    boost::asio::co_spawn(
                          ctx_.get_executor(),
                          [&conn, &resolver, params, hostname, this] {
                              return Database::coro_main(conn_, resolver, params, hostname);
                          },
                          [](std::exception_ptr ptr) {
                              if (ptr) {
                                  std::rethrow_exception(ptr);
                              }
                          });
    
    ctx_.run();
}
