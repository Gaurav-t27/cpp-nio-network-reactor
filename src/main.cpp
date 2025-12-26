#include <iostream>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <csignal>
#include <unistd.h>
#include <sys/eventfd.h>
#include "tcp_server.hpp"

// Global shutdown fd for signal handler
static int g_shutdown_fd = -1;

void signalHandler(int signal) {
    if((signal == SIGINT || signal == SIGTERM) && g_shutdown_fd != -1) {
        // Write to eventfd to signal shutdown (async-signal-safe)
        uint64_t val = 1;
        write(g_shutdown_fd, &val, sizeof(val));

    }
}

int main(int argc, char** argv) {
    try {
        int port = 8080;
        if (argc > 1) port = std::atoi(argv[1]);

        TCPServer server(port);
        
        // Set global shutdown fd for signal handler
        g_shutdown_fd = server.getShutdownFd();
        
        // Install signal handlers after we have the shutdown fd
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        std::cout << "Starting TCP server on port " << server.getPort() << "..." << std::endl;
        server.start();
        std::cout << "\nShutdown signal received. Stopping server..." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}