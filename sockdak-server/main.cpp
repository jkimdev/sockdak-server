//
//  main.cpp
//  sockdak-server
//
//  Created by Jimmy on 2023/10/28.
//

#include <iostream>
#include "Common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVERPORT 9000
#define BUFSIZE 512


int main(int argc, const char * argv[]) {
    int retval;
    
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) err_quit("socket()");
    
    // bind()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if(retval == -1) err_quit("bind()");
    
    // listen
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == -1) err_quit("listen()");
    
    // variables
    int client_sock = 0;
    struct sockaddr_in clientAddr;
    socklen_t addrlen;
    char buf[BUFSIZE + 1];
    
    while (1) {
        // aceept
        addrlen = sizeof(clientAddr);
        client_sock = accept(client_sock, (struct sockaddr *)&clientAddr, &addrlen);
        if (client_sock == -1) {
            err_display("accept()");
            break;
        }
    }
}
