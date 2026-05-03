#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <string>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/types.h>  // Recomended on socket manual (see manual NOTES)
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>  // inet_ntop()
#include <utility> // make_pair(t1, t2)

#include "Server.hpp"
#include "Command/Command.hpp"
#include "log.hpp"

// Maximum number of events
#define MAX_EVENTS 10

// Buffer size for each socket read. RFC messages are limited to 512 bytes,
#define SERVER_RECEPTION_CHUNCK_SIZE 1024

// From the RFC:
// > these messages shall not exceed 512 characters in length, counting all
// > characters including the trailing CR-LF
#define IRC_MAX_MESSAGE_SIZE         512

// From the RFC:
// > <prefix> ::= <servername> | <nick> | <extended prefix>
// > <servername> ::= <host>
// > <host>       ::= see RFC 952 [DNS:4] for details on allowed hostnames
// From RFC 952
// > <official hostname> ::= <hname>
// > <hname> ::= <name>*["."<name>]
// > <name>  ::= <let>[*[<let-or-digit-or-hyphen>]<let-or-digit>]
#define SERVER_HOSTNAME     "cbd.42porto.com"

LOG_REGISTER(server);

Server::Server(void) : port(666), passw("42"), srv_socket(-1), epollfd(-1) {}

Server::Server(unsigned int port, std::string passw) :
                        port(port), passw(passw),
                        srv_socket(-1), epollfd(-1) {}

Server::~Server(void) {
    if (this->srv_socket >= 0)
        close(this->srv_socket);
}

std::string Server::getHostname(void) const {
    return SERVER_HOSTNAME;
}

bool Server::checkPass(const std::string &passw) const {
    return (this->passw == passw);
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
                if (events[n].events & EPOLLIN)
                    this->receiveData(events[n].data.fd);
                // Skip fds that were disconnected while handling this epoll batch.
                // Any later EPOLLOUT for them is stale.
                if (this->client.find(events[n].data.fd) == this->client.end())
                    continue;
                if (events[n].events & EPOLLOUT)
                    this->flushReplyBuffer(events[n].data.fd);
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
        close(conn_sock);
        return EXIT_FAILURE;
    }
    this->connectClient(conn_sock);
    return 0;
}

void Server::addClientToNickList(Client &c, const std::string &oldname) {
    std::map<std::string, Client*>::iterator i = this->nickList.find(oldname);
    if (i != this->nickList.end()) {
        LOG_DBG("Updating the existing nick on the list");
        this->nickList.erase(oldname);
    }
    this->nickList[c.getNick()] = &c;
}

Client *Server::getClient(const std::string &nick) {
    if (this->nickList.find(nick) == this->nickList.end())
        return NULL;
    return this->nickList[nick];
}

void Server::connectClient(int fd) {
    this->client[fd] = Client(fd);
}

void Server::disconnectClient(int fd) {
    removeClientFromChannels(fd);
    std::map<int, Client>::iterator it = client.find(fd);
    if (it != client.end()) {
        std::string nick = it->second.getNick();
        if (!nick.empty())
            nickList.erase(nick);

        client.erase(it);
    }
    // epoll_ctl DEL is automatic on close. Check NOTES on epoll manual
    close(fd);
}

int Server::setWriteInterest(int fd, bool enabled) {
    struct epoll_event ev;

    ev.events = EPOLLIN | EPOLLET;
    if (enabled) {
        // Enable EPOLLOUT only while we have pending data.
        // Leaving it armed permanently would wake us up even when the socket is already writable.
        ev.events |= EPOLLOUT;
    }
    ev.data.fd = fd;
    return epoll_ctl(this->epollfd, EPOLL_CTL_MOD, fd, &ev);
}

int Server::flushReplyBuffer(int fd) {
    std::string &buffer = this->client[fd].getWriteBuf();

    while (!buffer.empty()) {
        ssize_t sent = send(fd, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
        if (sent > 0) {
            buffer.erase(0, static_cast<size_t>(sent));
        } else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        } else {
            LOG_ERR("send error on fd " << fd << ". errno = " << errno);
            this->disconnectClient(fd);
            return -1;
        }
    }

    // Disable EPOLLOUT once the output buffer is empty to avoid unnecessary wakeups.
    if (buffer.empty() && this->setWriteInterest(fd, false) == -1) {
        LOG_ERR("epoll_ctl MOD failed on fd " << fd << ". errno = " << errno);
        this->disconnectClient(fd);
        return -1;
    }

    return 0;
}

void Server::processBufferedMessages(int fd) {
    std::string &buffer = this->client[fd].getReadBuf();

    while (true) {
        std::string::size_type end = buffer.find("\r\n");
        if (end == std::string::npos)
            break;

        std::string message = buffer.substr(0, end + 2);
        buffer.erase(0, end + 2);

        if (message.length() > IRC_MAX_MESSAGE_SIZE - 2) {
            LOG_ERR("Message too long on fd " << fd << ", dropping it");
            continue;
        }

        Command *cmd = Command::parsing(message);
        if (cmd != NULL) {
            int ret = cmd->execute(*this, this->client[fd]);
            delete cmd;
            if (ret < 0)
                break;
        }
    }
}

int Server::sendPrivMsg(Client &source, Client &target, const std::string &msg) {
    std::string target_msg = "PRIVMSG " + target.getNick();
    return this->sendGenericMsg(source, target, target_msg, msg);
}

int Server::sendGenericMsg(Client &source, Client &target,
                           const std::string &target_msg, const std::string &msg) {
    std::stringstream ss;
    ss << ":" << source.getNick() <<
          " " << target_msg;
    if (!msg.empty())
        ss << " :" << msg;
    ss << "\r\n";

    std::string reply = ss.str();
    std::string &out = target.getWriteBuf();
    bool wasEmpty = out.empty();

    out.append(reply);
    int target_fd = target.getFd();
    if (this->flushReplyBuffer(target_fd) < 0)
        return -1;

    // If the buffer was empty before this reply and flushing left bytes queued,
    // re-arm EPOLLOUT so the kernel wakes us when the socket can write again.
    if (wasEmpty && !out.empty() && this->setWriteInterest(target_fd, true) == -1) {
        LOG_ERR("epoll_ctl MOD failed on fd " << target_fd << ". errno = " << errno);
        this->disconnectClient(target_fd);
        return -1;
    }
    return 0;
}

int Server::sendReply(int fd, int err, const std::string &cmd, const std::string &trailing) {
    //TODO: Move this Client server to internal attribute
    Client server(this->srv_socket, SERVER_HOSTNAME);
    std::stringstream ss;
    ss << err << " " << this->client[fd].getNick();
    if (!cmd.empty())
        ss << " " << cmd;
    std::string target_msg = ss.str();

    return this->sendGenericMsg(server, this->client[fd], target_msg, trailing);
}

int Server::receiveData(int fd) {
    ssize_t bytes;
    char buff[SERVER_RECEPTION_CHUNCK_SIZE];

    while (true) {
        // MSG_DONTWAIT not needed if socket is O_NONBLOCK
        bytes = recv(fd, buff, sizeof(buff), 0);

        if (bytes > 0) {
            this->client[fd].getReadBuf().append(buff, bytes);
            LOG_DBG("Received chunk (" << fd << ", size: " << bytes << ")");
        } else if (bytes == 0) {
            LOG_WRN("Client <" << fd << "> Disconnected");
            this->disconnectClient(fd);
            return 0;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Done reading all available data for this event trigger
                break;
            } else {
                // A real error occurred (e.g., ECONNRESET)
                int ret = errno;
                if (errno == ECONNRESET) {
                    LOG_INF("Client " << fd << " disconnected");
                    ret = 0;
                } else
                    LOG_ERR("recv error on fd " << fd << ". errno = " << errno);
                this->disconnectClient(fd);
                return ret;
            }
        }
    }
    if (this->client.find(fd) == this->client.end())
        return 0;

    this->processBufferedMessages(fd);
    return 0;
}

int Server::run(void) {
    int ret = openServerSocket();
    if (ret != 0)
        return ret;
    return listenEvents();
}

Channel* Server::getChannel(const std::string &name) {
    std::map<std::string, Channel>::iterator i = this->channels.find(name);
    if (i == this->channels.end())
        return NULL;
    return &(i->second);
}

Channel& Server::createChannel(const std::string &name) {
    std::pair<std::map<std::string, Channel>::iterator, bool> result;

    //  Insert the channel if it does not exist yet; otherwise get the existing one
    result = this->channels.insert(std::make_pair(name, Channel(name)));

    return result.first->second;
}

void Server::removeChannel(const std::string &name) {
    this->channels.erase(name);
}

void Server::removeClientFromChannels(int fd) {
    for(std::map<std::string, Channel>::iterator i = channels.begin();
        i != channels.end(); ++i) {
            if(i->second.isMember(fd))
                i->second.removeClient(fd);
        }
}

void Server::kickClient(int fd, Channel &ch) {
    ch.removeClient(fd);
}

void Server::broadcastMsg(Channel& ch, Client &sender, const std::string &cmd,
                          const std::string& msg, int excludeFd)
{
    const std::set<int> members = ch.getMembers();
    for (std::set<int>::iterator it = members.begin();
         it != members.end();
         ++it)
    {
        int fd = *it;
        if (fd == excludeFd)
            continue;
        this->sendGenericMsg(sender, this->client[*it], cmd, msg);
    }
}
