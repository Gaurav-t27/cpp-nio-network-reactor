#include <cstring>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include "../include/socket.hpp"
#include "../include/reactor.hpp"


// Test Socket RAII wrapper
TEST_CASE("Socket RAII basics", "[socket]") {
    
    SECTION("Socket creation and automatic cleanup") {
        {
            Socket s(socket(AF_INET, SOCK_STREAM, 0));
            REQUIRE(s.getFd() >= 0);
        }
        // Socket should be closed automatically when out of scope
    }
    
    SECTION("Move semantics work correctly") {
        Socket s1(socket(AF_INET, SOCK_STREAM, 0));
        int original_fd = s1.getFd();
        REQUIRE(original_fd >= 0);
        
        Socket s2(std::move(s1));
        REQUIRE(s1.getFd() == -1);  // s1 should be invalidated
        REQUIRE(s2.getFd() == original_fd);  // s2 should own the fd
    }
    
    SECTION("Move assignment works correctly") {
        Socket s1(socket(AF_INET, SOCK_STREAM, 0));
        int original_fd = s1.getFd();
        
        Socket s2;
        s2 = std::move(s1);
        
        REQUIRE(s1.getFd() == -1);
        REQUIRE(s2.getFd() == original_fd);
    }
}

TEST_CASE("Socket operations", "[socket]") {
    SECTION("Set non-blocking mode") {
        Socket s(socket(AF_INET, SOCK_STREAM, 0));
        REQUIRE(s.getFd() >= 0);
        
        REQUIRE_NOTHROW(s.setNonBlocking());
        
        // Verify non-blocking flag is set
        int flags = fcntl(s.getFd(), F_GETFL, 0);
        REQUIRE((flags & O_NONBLOCK) != 0);
    }
    
    SECTION("Set reuse address") {
        Socket s(socket(AF_INET, SOCK_STREAM, 0));
        REQUIRE(s.getFd() >= 0);
        
        REQUIRE_NOTHROW(s.setReuseAddr());
        
        // Verify SO_REUSEADDR is set
        int optval = 0;
        socklen_t optlen = sizeof(optval);
        getsockopt(s.getFd(), SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
        REQUIRE(optval == 1);
    }
    
}

TEST_CASE("Reactor basic operations", "[reactor]") {
    
    SECTION("Register and unregister handler") {
        Reactor reactor;
        
        // Create a test socket
        Socket s(socket(AF_INET, SOCK_STREAM, 0));
        REQUIRE(s.getFd() >= 0);
        
        int fd = s.getFd();
        bool handler_called = false;
        
        // Register handler
        REQUIRE_NOTHROW(reactor.registerHandler(fd, EPOLLIN, [&](int, uint32_t) {
            handler_called = true;
        }));
        
        // Unregister handler
        REQUIRE_NOTHROW(reactor.unregisterHandler(fd));
    }
    
    SECTION("Cannot register same fd twice") {
        Reactor reactor;
        Socket s(socket(AF_INET, SOCK_STREAM, 0));
        int fd = s.getFd();
        
        reactor.registerHandler(fd, EPOLLIN, [](int, uint32_t) {});
        
        // Second registration should throw
        REQUIRE_THROWS(reactor.registerHandler(fd, EPOLLIN, [](int, uint32_t) {}));
        
        reactor.unregisterHandler(fd);
    }
    
    SECTION("Modify handler event mask") {
        Reactor reactor;
        Socket s(socket(AF_INET, SOCK_STREAM, 0));
        int fd = s.getFd();
        
        reactor.registerHandler(fd, EPOLLIN, [](int, uint32_t) {});
        
        // Modify to add EPOLLOUT
        REQUIRE_NOTHROW(reactor.modifyHandler(fd, EPOLLIN | EPOLLOUT));
        
        reactor.unregisterHandler(fd);
    }
    
}

TEST_CASE("Reactor shutdown mechanism", "[reactor]") {
    SECTION("Shutdown fd is valid") {
        Reactor reactor;
        int shutdown_fd = reactor.getShutdownFd();
        REQUIRE(shutdown_fd >= 0);
        
        // Verify it's an eventfd
        uint64_t value = 1;
        ssize_t written = write(shutdown_fd, &value, sizeof(value));
        REQUIRE(written == sizeof(value));
    }
}


TEST_CASE("Reactor event handling with socket pair", "[reactor]") {
    SECTION("Handler is called when data available") {
        Reactor reactor;
        
        int sv[2];
        REQUIRE(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
        
        Socket s1(sv[0]);
        Socket s2(sv[1]);
        s1.setNonBlocking();
        s2.setNonBlocking();
        
        bool handler_called = false;
        uint32_t events_received = 0;
        
        reactor.registerHandler(s2.getFd(), EPOLLIN, [&](int fd, uint32_t events) {
            handler_called = true;
            events_received = events;
            
            char buffer[100];
            read(fd, buffer, sizeof(buffer));
            
            // Write to shutdown fd to stop reactor
            uint64_t value = 1;
            write(reactor.getShutdownFd(), &value, sizeof(value));
        });
        
        // Write data to trigger EPOLLIN
        const char* msg = "trigger";
        write(s1.getFd(), msg, strlen(msg));
        
        // Run reactor (will exit after handling the event due to shutdown write)
        reactor.run();
        
        REQUIRE(handler_called);
        REQUIRE((events_received & EPOLLIN) != 0);
        
        reactor.unregisterHandler(s2.getFd());
    }
}


int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
