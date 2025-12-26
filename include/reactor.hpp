#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <atomic>

class Reactor {
    using EventHandler = std::function<void(int, uint32_t)>;
    int m_epollFd;
    int m_shutdownFd;
    std::unordered_map<int, EventHandler> m_handlers;

public:
    Reactor();
    ~Reactor();

    void registerHandler(int fd, uint32_t events, EventHandler handler);
    void unregisterHandler(int fd);
    void modifyHandler(int fd, uint32_t events);
    void run();
    int getShutdownFd() const { return m_shutdownFd; }
};