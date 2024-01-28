//
//  main.cpp
//  sockdak-server
//
//  Created by Jimmy on 2023/10/28.
//

#include "server.hpp"
#include "database.hpp"

#define SERVER_PORT 9000
#define SERVER_ADDR "127.0.0.1"
#define USERNAME "root"
#define PASSWORD "password"
#define DB "sockdak_db"

int main(int argc, char* argv[]) {
    Server server(SERVER_ADDR, SERVER_PORT);
    Database database(USERNAME, PASSWORD, DB, SERVER_ADDR);
    server.Start();
    try {
        database.MainImpl(USERNAME, PASSWORD, DB);
        }
        catch (const boost::mysql::error_with_diagnostics& err) {
            std::cerr << "Error: " << err.what() << '\n'
                      << "Server diagnostics: " << err.get_diagnostics().server_message() << std::endl;
            return 1;
        }
        catch (const std::exception& err) {
            std::cerr << "Error: " << err.what() << std::endl;
            return 1;
        }
    return 0;
}
