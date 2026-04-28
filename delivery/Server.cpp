#include <cerrno>
#include <netinet/in.h>
#include <sys/types.h>  // Recomended on socket manual (see manual NOTES)
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h> // inet_ntop()
#include <sstream>
#include <cstdlib>

#include "Server.hpp"
#include "log.hpp"

// Maximum number of events
#define MAX_EVENTS 10

// Buffer size for each socket read. RFC messages are limited to 512 bytes,
#define SERVER_RECEPTION_CHUNCK_SIZE 1024

LOG_REGISTER(server);

Server::Server(void) : port(666), passw("42"), srv_socket(-1), epollfd(-1) {}

Server::Server(unsigned int port, std::string passw) :
                        port(port), passw(passw),
                        srv_socket(-1), epollfd(-1) {}

Server::~Server(void) {
    if (this->srv_socket >= 0)
        close(this->srv_socket);
}

int Server::openServerSocket(void) {
    this->srv_socket = socket(AF_INET, (SOCK_STREAM | SOCK_NONBLOCK), 0);
    if(this->srv_socket == -1) { //-> check if the socket is created
        LOG_ERR("Fail to create the socket");
        return -1;
    }
    // Check SO_REUSEADDR on man 7 socket
    int val = 1;
    int ret = setsockopt(this->srv_socket, SOL_SOCKET, SO_REUSEADDR,
                            &val, sizeof(val));
    if(ret == -1) {
        LOG_ERR("Fail to set socket option");
        return -1;
    }

    // From man 7 ip
    // INADDR_ANY (0.0.0.0) means any address for binding
    struct sockaddr_in add = {
        .sin_family = AF_INET,
        .sin_port = htons(this->port),
        .sin_addr = {
            .s_addr = INADDR_ANY,
        },
        .sin_zero = { 0 },
    };
    if (bind(this->srv_socket, (struct sockaddr *)&add, sizeof(add)) == -1) {
        LOG_ERR("faild to bind socket");
        return -1;
    }
    // listen for incoming connections and making the socket a passive socket
    if (listen(this->srv_socket, SOMAXCONN) == -1) {
        LOG_ERR("faild to bind socket");
        return -1;
    }
    return 0;
}

// This code is based on the epoll manual usage example
int Server::listenEvents(void) {
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds;

    this->epollfd = epoll_create1(0);
    if (this->epollfd == -1) {
        LOG_ERR("Error on epoll_create1");
        return EXIT_FAILURE;
    }
    ev.events = EPOLLIN;
    ev.data.fd = this->srv_socket;
    if (epoll_ctl(this->epollfd, EPOLL_CTL_ADD, this->srv_socket, &ev) == -1) {
        LOG_ERR("epoll_ctl: this->srv_socket");
        return EXIT_FAILURE;
    }

    for (;;) {
        nfds = epoll_wait(this->epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            LOG_ERR("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == this->srv_socket) {
                this->addNewClient();
            } else {
                this->receiveData(events[n].data.fd);
            }
        }
    }
    return 0;
}

int Server::addNewClient(void) {
    int conn_sock;
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    conn_sock = accept4(this->srv_socket,
                        (struct sockaddr *) &addr, &addrlen,
                        SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (conn_sock == -1) {
        LOG_ERR("Error on accept");
        return EXIT_FAILURE;
    }

    if (LOG_IS_INF_ENABLED) {
        // Print the ip and port of the connection
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN);
        std::stringstream ss;
        ss << "Client: " << conn_sock << ", addr = " << str << ":" << ntohs(addr.sin_port);
        std::string msg = ss.str();
        LOG_INF(msg);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = conn_sock;
    if (epoll_ctl(this->epollfd, EPOLL_CTL_ADD, conn_sock,
                  &ev) == -1) {
        LOG_ERR("epoll_ctl: conn_sock");
        return EXIT_FAILURE;
    }
    return 0;
}

int Server::receiveData(int fd) {
    char buff[SERVER_RECEPTION_CHUNCK_SIZE];
    ssize_t total = 0;
    ssize_t bytes;

    //TODO: It assumes that we will read all the bytes into this buffer, and it will contain
    // only one Command! Check if it is enough
    // From the RFC:
    // > IRC messages are always lines of characters terminated with a CR-LF
    // > (Carriage Return - Line Feed) pair, and these messages shall not
    // > exceed 512 characters in length, counting all characters including
    // > the trailing CR-LF. Thus, there are 510 characters maximum allowed
    // > for the command and its parameters.  There is no provision for
    // > continuation message lines.  See section 7 for more details about
    // > current implementations.
    //
    // TODO: Add bufferfull check for logging and drop connection
    while (true) {
        // MSG_DONTWAIT not needed if socket is O_NONBLOCK
        bytes = recv(fd, &buff[total], sizeof(buff) - 1 - total, 0);

        if (bytes > 0) {
            // Data received successfully
            buff[total + bytes] = '\0';
            LOG_DBG("Received msg (" << fd << ", size: " << bytes << "): " << &buff[total]);
            total += bytes;
        } else if (bytes == 0) {
            LOG_WRN("Client <" << fd << "> Disconnected");
            // epoll_ctl DEL is automatic on close. Check NOTES on epoll manual
            close(fd);
            return 0;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Done reading all available data for this event trigger
                break;
            } else {
                // A real error occurred (e.g., ECONNRESET)
                LOG_ERR("recv error on fd " << fd);
                close(fd);
                return EXIT_FAILURE;
            }
        }
    }
    if (total == 0)
        return 0;
    return 0;
}

int Server::run(void) {
    int ret = openServerSocket();
    if (ret != 0)
        return ret;
    return listenEvents();
}
