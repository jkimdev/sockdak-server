//
//  server.cpp
//  sockdak-server
//
//  Created by 김재민 on 2023/12/29.
//

#include "server.hpp"

Server::Server(std::string ip_address, unsigned short port_num) :
work(new io_service::work(ios)),
ep(ip::address::from_string(ip_address), port_num),
gate(ios, ep.protocol()) {}

void Server::Start() {
    for (int i = 0; i < THREAD_SIZE; i++)
        threadGroup.create_thread(std::bind(&Server::WorkerThread, this));
    std::this_thread::sleep_for(boost::asio::chrono::milliseconds(100));
    std::cout << "Threads Created" << std::endl;
    ios.post(std::bind(&Server::OpenGate, this));
    
    threadGroup.join();
}


void Server::WorkerThread() {
    lock.lock();
    std::cout << "[" << std::this_thread::get_id() << "]" << " Thread Start" << std::endl;
    lock.unlock();
    
    ios.run();
}

void Server::OpenGate() {
    boost::system::error_code ec;
    gate.bind(ep, ec);
    
    if (ec) {
        std::cout << "bind failed: " << ec.message() << std::endl;
        return;
    }
    gate.listen();
    std::cout << "Gate listening" << std::endl;
    
    StartAccept();
    
}

void Server::StartAccept() {
    Session* session = new Session();
    std::shared_ptr<tcp::socket> sock(new tcp::socket(ios));
    
    session->sock = sock;
    gate.async_accept(*sock, session->ep, std::bind(&Server::OnAccept, this, std::placeholders::_1, session));
}

void Server::OnAccept(const boost::system::error_code &ec, Session* session) {
    if (ec) {
        std::cout << "connect failed: " << ec.message() << std::endl;
        return;
    }
    
    lock.lock();
    sessions.push_back(session);
    lock.unlock();
    
    ios.post(std::bind(&Server::Receive, this, session));
    
    StartAccept();
    
}

void Server::SendData(Session* session) {
    session->sock->async_write_some(boost::asio::buffer(session->sbuf, sizeof(session->sbuf)), std::bind(&Server::OnSend, this, std::placeholders::_1, std::placeholders::_2));
}

void Server::OnSend(const boost::system::error_code& ec, std::size_t bytes_transferred){
    if (ec) {
        std::cout << "[" << std::this_thread::get_id() << "] async_write_some failed: " << ec.message() << bytes_transferred << std::endl;
        return;
    }
}

void Server::Receive(Session* session) {
    boost::system::error_code ec;
    
    size_t size;
    size = session->sock->read_some(boost::asio::buffer(session->buf, sizeof(session->buf)), ec);
    
    
    if (ec) {
        std::cout << "[" << std::this_thread::get_id() << "] read failed: " << ec.message() << std::endl;
        return;
    }
    
    lock.lock();
    session->buf[size] = '\0';
    session->rbuf = session->buf;
    std::cout << buf << std::endl;
    lock.unlock();
    
    DataManager(session);
    
    Receive(session);
}

void Server::SendToAll(Session* session, const std::string& message) {
    for (int i = 0; i < sessions.size(); i++) {
        if (session->sock != sessions[i]->sock) {
            sessions[i]->sbuf = "[" + session->user_name + "] " + message;
            SendData(sessions[i]);
        }
    }
}

void Server::DataManager(Session* session) {
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
    SendToAll(session, session->rbuf);
}

ACTION Server::ActionManager(std::string message) {
    std::string action = message.substr(0, message.length());
    if (action.compare("\friends")) {
        return FRIENDS;
    }
    return INVALID;
}

std::string Server::GetFriendList(Session* session) {
    std::string result;
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


