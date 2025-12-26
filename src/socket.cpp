#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <system_error>
#include <cerrno>
#include <cstring>
#include "socket.hpp"


Socket::Socket() : m_fd(-1) {
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }
}

Socket::Socket(int fd) : m_fd(fd) {
}

Socket::Socket(Socket&& other) noexcept : m_fd(other.m_fd) {
    other.m_fd = -1;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (m_fd != -1) {
            close(m_fd);
        }
        m_fd = other.m_fd;
        other.m_fd = -1;
    }
    return *this;
}

Socket::~Socket() {
    if (m_fd != -1) {
        close(m_fd);
    }
}

void Socket::setNonBlocking() {
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get socket flags");
    }
    if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set socket flags");
    }
}

void Socket::setReuseAddr() {
    int opt = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("Failed to set SO_REUSEADDR");
    }
}

void Socket::bind(int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(m_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Socket::listen() {
    if (::listen(m_fd, SOMAXCONN) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }
}

int Socket::getPort() const {
    if (m_fd == -1) return 0;
    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getsockname(m_fd, reinterpret_cast<sockaddr*>(&addr), &len) == -1) {
        throw std::system_error(errno, std::generic_category(), std::string("getsockname failed: ") + std::strerror(errno));
    }
    return ntohs(addr.sin_port);
}

