#pragma once

class Socket {
    int m_fd;
public:
    Socket();

    explicit Socket(int fd);

    Socket(const Socket&) = delete;

    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;

    Socket& operator=(Socket&& other) noexcept;

    ~Socket();  

    void setNonBlocking();

    void setReuseAddr();

    void bind(int port);  // Binding logic to be implemented

    void listen();  // Listening logic to be implemented

    int getPort() const;

    int getFd() const { return m_fd; }
};