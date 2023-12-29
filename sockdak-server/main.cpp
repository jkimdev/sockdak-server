//
//  main.cpp
//  sockdak-server
//
//  Created by Jimmy on 2023/10/28.
//

#include <boost/asio.hpp>
#include <iostream>

#define SERVER_PORT 9000

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

class Server {
    io_service ios;
    shared_ptr<io_service::work> work;
    tcp::endpoint ep;
    tcp::acceptor gate;
    boost::asio::detail::thread_group threadGroup;
    boost::asio::detail::mutex lock;
    vector<Session*> sessions;
    const int THREAD_SIZE = 4;
    enum ACTION { FRIENDS, INVALID };
    
public:
    Server(string ip_address, unsigned short port_num) :
    work(new io_service::work(ios)),
    ep(ip::address::from_string(ip_address), port_num),
    gate(ios, ep.protocol()) {}
    
    void Start() {
        for (int i = 0; i < THREAD_SIZE; i++)
            threadGroup.create_thread(bind(&Server::WorkerThread, this));
        this_thread::sleep_for(boost::asio::chrono::milliseconds(100));
        cout << "Threads Created" << endl;
        ios.post(bind(&Server::OpenGate, this));
        
        threadGroup.join();
    }
    
private:
    char buf[80];
    
    void WorkerThread() {
        lock.lock();
        cout << "[" << this_thread::get_id() << "]" << " Thread Start" << endl;
        lock.unlock();
        
        ios.run();
    }
    
    void OpenGate() {
        boost::system::error_code ec;
        gate.bind(ep, ec);
        
        if (ec) {
            cout << "bind failed: " << ec.message() << endl;
            return;
        }
        gate.listen();
        cout << "Gate listening" << endl;
        
        StartAccept();
        
    }
    
    void StartAccept() {
        Session* session = new Session();
        shared_ptr<tcp::socket> sock(new tcp::socket(ios));
        
        session->sock = sock;
        gate.async_accept(*sock, session->ep, bind(&Server::OnAccept, this, std::placeholders::_1, session));
    }
    
    void OnAccept(const boost::system::error_code &ec, Session* session) {
        if (ec) {
            cout << "connect failed: " << ec.message() << endl;
            return;
        }
        
        lock.lock();
        sessions.push_back(session);
        lock.unlock();
        
        ios.post(bind(&Server::Receive, this, session));
        
        StartAccept();
        
    }
    
    void Receive(Session* session) {
        boost::system::error_code ec;
        
        size_t size;
        size = session->sock->read_some(boost::asio::buffer(session->buf, sizeof(session->buf)), ec);
        
        
        if (ec) {
            cout << "[" << this_thread::get_id() << "] read failed: " << ec.message() << endl;
            return;
        }
        
        lock.lock();
        session->buf[size] = '\0';
        session->rbuf = session->buf;
        cout << buf << endl;
        lock.unlock();
        
        DataManager(session);
        
        Receive(session);
    }
    
    ACTION ActionManager(string message) {
        string action = message.substr(0, message.length());
        if (action.compare("\friends")) {
            return FRIENDS;
        }
        return INVALID;
    }
    
    void DataManager(Session* session) {
        if (session->buf[0] == '/') {
            ACTION action = ActionManager(session->rbuf);

            switch (action) {
                case ACTION::FRIENDS:
                    session->sbuf = GetFriendList(session);
                    break;
                case ACTION::INVALID:
                    session->sbuf = "INVALID ACTION";
                    break;
            }
            SendData(session);
            return;
        }
        SendToAll(session, session->sbuf);
    }
    
    string GetFriendList(Session* session) {
        string result;
        if (sessions.size() < 2) {
            result = "No friends are connected";
        }
        else {
            for(int i = 0; i < sessions.size(); i++) {
                if (session->sock != sessions[i]->sock) {
                    result += sessions[i]->user_name + "\n";
                }
            }
        }
        return result;
    }
    
    void SendData(Session* session) {
        session->sock->async_write_some(boost::asio::buffer(session->sbuf, sizeof(session->sbuf)), bind(&Server::OnSend, this, std::placeholders::_1, std::placeholders::_2));
    }
    
    void SendToAll(Session* session, const string& message) {
        for (int i = 0; i < sessions.size(); i++) {
            if (session->sock != sessions[i]->sock) {
                sessions[i]->sbuf = message;
                SendData(sessions[i]);
            }
        }
    }
    
    void OnSend(const boost::system::error_code& ec, std::size_t bytes_transferred){
        if (ec) {
            cout << "[" << this_thread::get_id() << "] async_write_some failed: " << ec.message() << bytes_transferred << endl;
            return;
        }
    }
};

int main(int argc, char* argv[]) {
    Server server(address_v4::any().to_string(), SERVER_PORT);
    server.Start();
    return 0;
}
