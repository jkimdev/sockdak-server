//
//  main.cpp
//  sockdak-server
//
//  Created by Jimmy on 2023/10/28.
//

#include "server.hpp"

#define SERVER_PORT 9000

int main(int argc, char* argv[]) {
    Server server(address_v4::any().to_string(), SERVER_PORT);
    server.Start();
    return 0;
}
