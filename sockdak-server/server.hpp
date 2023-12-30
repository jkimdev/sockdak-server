//
//  server.hpp
//  sockdak-server
//
//  Created by 김재민 on 2023/12/29.
//

#ifndef server_hpp
#define server_hpp

#include <boost/asio.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

struct Session
{
    shared_ptr<ip::tcp::socket> sock;
    ip::tcp::endpoint ep;
    
    string user_name = "UNKNOWN";
    int room_no = -1;
    
    string sbuf;
    string rbuf;
    char buf[512];
};

enum ACTION { FRIENDS, INVALID };

class Server {
    io_service ios;
    shared_ptr<io_service::work> work;
    tcp::endpoint ep;
    tcp::acceptor gate;
    boost::asio::detail::thread_group threadGroup;
    boost::asio::detail::mutex lock;
    vector<Session*> sessions;
    const int THREAD_SIZE = 4;
    
public:
    Server(string, unsigned short);
    
    void Start();
    
private:
    char buf[512];
    
    void WorkerThread();
    
    void OpenGate();
    
    void StartAccept();
    
    void OnAccept(const boost::system::error_code &ec, Session* session);
    
    void SendData(Session* session);
    
    void OnSend(const boost::system::error_code& ec, std::size_t bytes_transferred);
    
    void Receive(Session* session);
    
    void SendToAll(Session* session, const string& message);
    
    void DataManager(Session* session);
    
    ACTION ActionManager(string message);
    
    string GetFriendList(Session* session);
    
};

#endif /* server_hpp */
