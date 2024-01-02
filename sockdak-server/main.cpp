//
//  main.cpp
//  sockdak-server
//
//  Created by Jimmy on 2023/10/28.
//

#include "server.hpp"

#define SERVER_PORT 9000
#define SERVER_ADDR "127.0.0.1"

int main(int argc, char* argv[]) {
    Server server(SERVER_ADDR, SERVER_PORT);
    server.Start();
    return 0;
}
