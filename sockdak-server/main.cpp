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

class Server {
    io_service ios;
    shared_ptr<io_service::work> work;
    tcp::endpoint ep;
    tcp::acceptor gate;
    boost::asio::detail::thread_group threadGroup;
    boost::asio::detail::mutex lock;
    const int THREAD_SIZE = 4;
    
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
        
        if (ec)
        {
            cout << "bind failed: " << ec.message() << endl;
            return;
        }
        gate.listen();
        cout << "Gate listening" << endl;
        
        StartAccept();
        
    }
    
    void StartAccept() {
        shared_ptr<tcp::socket> sock(new tcp::socket(ios));
        
        gate.async_accept(*sock, [this, sock](const boost::system::error_code& ec) {
            if (ec) {
                cout << "connect failed: " << ec.message() << endl;
                return;
            }
//            lock.lock();
//            cout << "[" << this_thread::get_id() << "]" << " Client Accepted" << endl;
//            lock.unlock();
            
            ios.post([this, sock]() {
                Receive(*sock);
            });
            
            StartAccept();
        });
    }
    
    void Receive(tcp::socket& socket) {
        boost::system::error_code ec;
        
        size_t size;
        size = socket.read_some(boost::asio::buffer(buf, sizeof(buf)), ec);
        
        
        if (ec) {
            cout << "[" << this_thread::get_id() << "] read failed: " << ec.message() << endl;
            return;
        }
        
        lock.lock();
        buf[size] = '\0';
        cout << buf << endl;
        lock.unlock();
        
        SendData(socket, buf);
        
        Receive(socket);
    }
        
    void SendData(tcp::socket& socket, const string& message) {
        socket.async_write_some(boost::asio::buffer(message, sizeof(message)), bind(&Server::OnSend, this, std::placeholders::_1, std::placeholders::_2));
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
