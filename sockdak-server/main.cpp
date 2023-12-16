//
//  main.cpp
//  sockdak-server
//
//  Created by Jimmy on 2023/10/28.
//

#include <boost/asio.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

string getData(tcp::socket& socket) {
    boost::asio::streambuf buf;
    read_until(socket, buf, "\n");
    string data = buffer_cast<const char*>(buf.data());
    return data;
}

void sendData(tcp::socket& socket, const string& message) {
    write(socket, buffer(message + "\n"));
}

int main(int argc, char* argv[]) {
    io_service io_service;
    
    tcp::acceptor acceptor_server(io_service, tcp::endpoint(tcp::v4(), 9999));
    
    tcp::socket server_socket(io_service);
    
    acceptor_server.accept(server_socket);
    
    string username = getData(server_socket);
    username.pop_back();
    
    string response, reply;
    reply = "Hello, " + username + "!";
    cout << "Server: " << reply << endl;
    sendData(server_socket, reply);
    
    while (true) {
        response = getData(server_socket);
        response.pop_back();
        
        if (response == "exit") {
            cout << "bye!" << endl;
            break;
        }
        cout << username << ": " << response << endl;
        cout << "Server" << ": ";
        getline(cin, reply);
        
        if (reply == "exit")
            break;
    }
    return 0;
}
