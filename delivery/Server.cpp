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

LOG_REGISTER(server);

Server::Server(void) : port(666), passw("42"), srv_socket(-1) {}

Server::Server(unsigned int port, std::string passw) : port(port), passw(passw), srv_socket(-1) {
}

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

int Server::listenEvents(void) {
    #define MAX_EVENTS 10
    struct epoll_event ev, events[MAX_EVENTS];
    int conn_sock, nfds, epollfd;

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        LOG_ERR("Error on epoll_create1");
        return EXIT_FAILURE;
    }
    ev.events = EPOLLIN;
    ev.data.fd = this->srv_socket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, this->srv_socket, &ev) == -1) {
        LOG_ERR("epoll_ctl: this->srv_socket");
        return EXIT_FAILURE;
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            LOG_ERR("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == this->srv_socket) {
                sockaddr_in addr;
                socklen_t addrlen;
                conn_sock = accept(this->srv_socket,
                                   (struct sockaddr *) &addr, &addrlen);
                if (conn_sock == -1) {
                    LOG_ERR("Error on accept");
                    return EXIT_FAILURE;
                }

                //@@@@ DEBUG
                char str[INET_ADDRSTRLEN];
                // now get it back and print it
                inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN);
                std::stringstream ss;
                ss << "addr = " << str << ":" << ntohs(addr.sin_port);
                std::string msg = ss.str();
                LOG_INF(msg);
                // end @@@@@

                //setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                              &ev) == -1) {
                    LOG_ERR("epoll_ctl: conn_sock");
                    return EXIT_FAILURE;
                }
            } else {
                // do_use_fd(events[n].data.fd);
            }
        }
    }
    return 0;
}

int Server::run(void) {
    int ret = openServerSocket();
    if (ret != 0)
        return ret;
    return listenEvents();
}
