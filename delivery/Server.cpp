#include <cstddef>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
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

// Per-recv() chunk size. This is just how much we ask the kernel for in one
// call; it does NOT cap accumulated unframed input. That cap lives in
// utils/IrcParse::tokenizeNextMessage (drops the buffer if it grows past
// the RFC line size before a CRLF arrives).
#define SERVER_RECEPTION_CHUNCK_SIZE 1024

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

Server::Server(void) : port(666), passw("42"), nextSessionId(1), srv_socket(-1), epollfd(-1), shutdown_flag(NULL) {}

Server::Server(unsigned int port, std::string passw) :
                        port(port), passw(passw),
                        nextSessionId(1),
                        srv_socket(-1), epollfd(-1), shutdown_flag(NULL) {}

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
        return EXIT_FAILURE;
    }
    // Check SO_REUSEADDR on man 7 socket
    int val = 1;
    int ret = setsockopt(this->srv_socket, SOL_SOCKET, SO_REUSEADDR,
                            &val, sizeof(val));
    if(ret == -1) {
        LOG_ERR("Fail to set socket option");
        return EXIT_FAILURE;
    }

    // From man 7 ip
    // INADDR_ANY (0.0.0.0) means any address for binding
    struct sockaddr_in add;
    std::memset(&add, 0, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(this->port);
    add.sin_addr.s_addr = INADDR_ANY;
    if (bind(this->srv_socket, (struct sockaddr *)&add, sizeof(add)) == -1) {
        LOG_ERR("faild to bind socket");
        return EXIT_FAILURE;
    }
    // listen for incoming connections and making the socket a passive socket
    if (listen(this->srv_socket, SOMAXCONN) == -1) {
        LOG_ERR("faild to bind socket");
        return EXIT_FAILURE;
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

    int ret = 0;
    while (!*(this->shutdown_flag)) {
        nfds = epoll_wait(this->epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR)
                continue;
            LOG_ERR("epoll_wait");
            ret = EXIT_FAILURE;
            break;
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
    if (ret == 0)
        LOG_INF("Shutdown signal received, cleaning up...");
    else
        LOG_ERR("Exiting due to epoll_wait error, cleaning up...");

    this->shutdown(); // Notify Clients and shutdown
    return ret;
}

int Server::addNewClient(void) {
    int conn_sock;
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    conn_sock = accept(this->srv_socket,
                       (struct sockaddr *) &addr, &addrlen);
    if (conn_sock == -1) {
        LOG_ERR("Error on accept");
        return EXIT_FAILURE;
    }
    if (fcntl(conn_sock, F_SETFL, O_NONBLOCK) == -1) {
        LOG_ERR("fcntl O_NONBLOCK failed on conn_sock. errno = " << errno);
        close(conn_sock);
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
    ev.events = EPOLLIN;
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

Client *Server::getClientByFd(int fd) {
    std::map<int, Client>::iterator i = this->client.find(fd);
    if (i == this->client.end())
        return NULL;
    return &i->second;
}

void Server::connectClient(int fd) {
    Client c(fd, this->nextSessionId++);
    this->client[fd] = c;
}

void Server::disconnectClient(int fd) {
    // Idempotent: a write failure inside a command may have already
    // disconnected the same fd via flushReplyBuffer. Subsequent callers
    // (e.g. receiveData propagating a negative handler return) hit this
    // early-return instead of double-closing.
    std::map<int, Client>::iterator it = client.find(fd);
    if (it == client.end())
        return;

    removeInvitesFromChannels(it->second.getSessionId());
    removeClientFromChannels(fd);
    std::string nick = it->second.getNick();
    if (!nick.empty())
        nickList.erase(nick);
    client.erase(it);
    // epoll_ctl DEL is automatic on close. Check NOTES on epoll manual
    close(fd);
}

int Server::setWriteInterest(int fd, bool enabled) {
    struct epoll_event ev;

    ev.events = EPOLLIN;
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

    if (!buffer.empty()) {
        // Single send per call. Level-triggered EPOLLOUT will fire again
        // if the kernel buffer fills mid-write and bytes remain queued.
        ssize_t sent = send(fd, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
        if (sent > 0) {
            buffer.erase(0, static_cast<size_t>(sent));
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

int Server::processBufferedMessages(int fd) {
    int ret = 0;
    std::string &buffer = this->client[fd].getReadBuf();

    while (true) {
        IrcMessage msg;
        TokenizeResult r = tokenizeNextMessage(buffer, msg);
        if (r == TOKENIZE_NO_FRAME)
            break;
        if (r == TOKENIZE_TOO_LONG) {
            LOG_ERR("Message too long on fd " << fd << ", dropping it");
            continue;
        }
        if (r == TOKENIZE_MALFORMED) {
            LOG_ERR("Malformed message on fd " << fd << ", dropping it");
            continue;
        }
        if (r == TOKENIZE_EMPTY)
            continue;

        Command *cmd = Command::fromMessage(msg);
        if (cmd != NULL) {
            ret = cmd->execute(*this, this->client[fd]);
            delete cmd;
            if (ret < 0)
                break;
        }
    }
    return ret;
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
    Client server(this->srv_socket, SERVER_HOSTNAME, 0);
    std::stringstream ss;
    ss << err << " " << this->client[fd].getNick();
    if (!cmd.empty())
        ss << " " << cmd;
    std::string target_msg = ss.str();

    return this->sendGenericMsg(server, this->client[fd], target_msg, trailing);
}

int Server::receiveData(int fd) {
    char buff[SERVER_RECEPTION_CHUNCK_SIZE];

    // Single recv per event. Level-triggered EPOLLIN will fire again on the
    // next epoll_wait() if more data is buffered in the kernel.
    ssize_t bytes = recv(fd, buff, sizeof(buff), 0);

    if (bytes > 0) {
        this->client[fd].getReadBuf().append(buff, bytes);
        LOG_DBG("Received chunk (" << fd << ", size: " << bytes << ")");
    } else if (bytes == 0) {
        LOG_WRN("Client <" << fd << "> Disconnected");
        this->disconnectClient(fd);
        return 0;
    } else {
        LOG_ERR("recv error on fd " << fd << ". errno = " << errno);
        this->disconnectClient(fd);
        return -1;
    }

    int ret = this->processBufferedMessages(fd);
    if (ret < 0)
        this->disconnectClient(fd);
    return ret;
}

int Server::run(volatile sig_atomic_t *shutdown_flag) {
    this->shutdown_flag = shutdown_flag;
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

void Server::removeInvitesFromChannels(std::size_t sessionId) {
    for (std::map<std::string, Channel>::iterator i = channels.begin();
         i != channels.end(); ++i) {
        i->second.removeInvite(sessionId);
    }
}

void Server::kickClient(int fd, Channel &ch) {
    ch.removeClient(fd);
}

int Server::broadcastMsg(Channel& ch, Client &sender, const std::string &cmd,
                         const std::string& msg, int excludeFd)
{
    // if the caller didn't explicitly exclude
    // sender, sender gets the echo (e.g. JOIN echoes to the joiner).
    bool echo = (excludeFd != sender.getFd());
    return this->broadcastTo(ch.getMembers(), sender, cmd, msg, excludeFd, echo);
}

int Server::broadcastTo(const std::set<int> &fds, Client &sender,
                        const std::string &cmd, const std::string &msg,
                        int excludeFd, bool echoToSender)
{
    // Snapshot the recipient set so a mid-loop disconnect (which can erase
    // entries from the caller's underlying set, e.g. a Channel::members)
    // cannot invalidate our iterator.
    std::set<int> snapshot(fds);
    // Sender is always skipped inside the loop: if a self-send failed
    // mid-iteration it would disconnect sender, leaving the `sender`
    // reference dangling for the remaining peer sends. Self-echo is
    // performed once after the loop, so it's the last thing we do.
    int senderFd = sender.getFd();
    for (std::set<int>::iterator it = snapshot.begin();
         it != snapshot.end();
         ++it)
    {
        int fd = *it;
        if (fd == excludeFd || fd == senderFd)
            continue;
        std::map<int, Client>::iterator ci = this->client.find(fd);
        if (ci == this->client.end())
            continue;
        this->sendGenericMsg(sender, ci->second, cmd, msg);
    }
    if (echoToSender)
        return this->sendGenericMsg(sender, sender, cmd, msg);
    return 0;
}

std::set<int> Server::getCommonChannelFds(int fd) const {
    std::set<int> result;
    for (std::map<std::string, Channel>::const_iterator i = this->channels.begin();
         i != this->channels.end(); ++i)
    {
        if (!i->second.isMember(fd))
            continue;
        const std::set<int>& members = i->second.getMembers();
        result.insert(members.begin(), members.end());
    }
    return result;
}

void Server::shutdown(void) {
    for (std::map<int, Client>::iterator it = this->client.begin();
         it != this->client.end(); ++it)
    {
        std::string msg = "ERROR :Closing Link: " +
                          this->getHostname() +
                          " (Server shutting down)\r\n";
        send(it->first, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        close(it->first);
    }
    this->client.clear();
    this->nickList.clear();
    this->channels.clear();

    if (this->epollfd >= 0) {
        close(this->epollfd);
        this->epollfd = -1;
    }
}
