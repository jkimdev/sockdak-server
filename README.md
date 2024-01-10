# SOCKDAK-SERVER

### CODE


-   `Server` 클래스의 생성자입니다. IP 주소와 포트 번호를 입력으로 받아서 서버를 초기화합니다.
```cpp
Server::Server(std::string ip_address, unsigned short port_num) :
work(new io_service::work(ios)),
ep(ip::address::from_string(ip_address), port_num),
gate(ios, ep.protocol()) {}
```
-   `Start` 함수는 서버를 시작하는 함수로, 작업 스레드를 생성하고 게이트를 열어 클라이언트의 연결을 기다립니다.
```cpp
void Server::Start() {
    for (int i = 0; i < THREAD_SIZE; i++)
        threadGroup.create_thread(std::bind(&Server::WorkerThread, this));
    std::this_thread::sleep_for(boost::asio::chrono::milliseconds(100));
    std::cout << "Threads Created" << std::endl;
    ios.post(std::bind(&Server::OpenGate, this));
    
    threadGroup.join();
}
```
-   `WorkerThread` 함수는 각 작업 스레드에서 실행되는 함수로, `io_service`의 `run` 함수를 호출하여 비동기 작업을 실행합니다.
```cpp
void Server::WorkerThread() { 
lock.lock(); 
std::cout << "[" << std::this_thread::get_id() << "]" << " Thread Start" << std::endl; lock.unlock(); 
ios.run(); 
}
```
-   `WorkerThread` 함수는 각 작업 스레드에서 실행되는 함수로, `io_service`의 `run` 함수를 호출하여 비동기 작업을 실행합니다.
```cpp
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
```
-   `OpenGate` 함수는 서버의 게이트를 열고 클라이언트의 연결을 수신 대기 상태로 설정합니다.
```cpp
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
```
-   `StartAccept` 함수는 클라이언트의 연결을 수락하기 위해 비동기 `async_accept`를 호출합니다.
```cpp
void Server::StartAccept() {
    Session* session = new Session();
    std::shared_ptr<tcp::socket> sock(new tcp::socket(ios));
    
    session->sock = sock;
    gate.async_accept(*sock, session->ep, std::bind(&Server::OnAccept, this, std::placeholders::_1, session));
}
```
-   `OnAccept` 함수는 클라이언트 연결이 수락되면 실행되며, 새로운 클라이언트 세션을 생성하고 연결된 클라이언트를 관리합니다.
```cpp
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
```
-   `SendData` 함수는 클라이언트에 데이터를 비동기적으로 전송합니다.
```cpp
void Server::SendData(Session* session) {
    session->sock->async_write_some(boost::asio::buffer(session->sbuf, sizeof(session->sbuf)), std::bind(&Server::OnSend, this, std::placeholders::_1, std::placeholders::_2));
}
```
-   `Receive` 함수는 클라이언트로부터 데이터를 비동기적으로 수신하고, 데이터를 처리하는 역할을 합니다.
```cpp
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
```
