#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
#include <iostream>
#include "socket.hpp"
#include "reactor.hpp"



struct ClientState {
    std::unique_ptr<Socket> socket;
    std::vector<char> write_buffer;
};

class TCPServer {
    Socket m_listen_socket;
    Reactor m_reactor;
    std::unordered_map<int, ClientState> m_clients;
    static constexpr size_t MAX_WRITE_BUFFER_SIZE = 64 * 1024; // 64KB threshold
    static constexpr size_t RESUME_WRITE_BUFFER_SIZE = 32 * 1024; // Resume at 32KB
public:
    TCPServer(int port);

    void start();

    int getPort() const;
    int getShutdownFd() const { return m_reactor.getShutdownFd(); }
private:
    void handleNewConnection(int fd);
    void handleClientData(int fd);
    void handleClientWrite(int fd);
    void cleanupClient(int fd);
};