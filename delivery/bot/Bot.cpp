/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djunho <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:16 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:16 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include "Bot.hpp"
#include "../log.hpp"

LOG_REGISTER(bot);

#define BOT_SITE_URL  "https://github.com/djunho/ft_irc"
#define BOT_RECV_CHUNK 1024

Bot::Bot(const std::string &host, unsigned int port,
         const std::string &password, const std::string &nick,
         const std::string &channel)
    : host(host), port(port), password(password), nick(nick), channel(channel),
      fd(-1), read_buf(), write_buf(), shutdown_flag(NULL) {}

Bot::Bot(const Bot &other)
    : host(other.host), port(other.port), password(other.password),
      nick(other.nick), channel(other.channel),
      fd(-1), read_buf(other.read_buf), write_buf(other.write_buf),
      shutdown_flag(other.shutdown_flag) {}

Bot &Bot::operator=(const Bot &other) {
    if (this != &other) {
        // fd is intentionally not copied — it stays owned by the original
        host = other.host;
        port = other.port;
        password = other.password;
        nick = other.nick;
        channel = other.channel;
        read_buf = other.read_buf;
        write_buf = other.write_buf;
        shutdown_flag = other.shutdown_flag;
    }
    return *this;
}

Bot::~Bot(void) {
    if (fd >= 0)
        close(fd);
}

int Bot::connectToServer(void) {
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), NULL, &hints, &res) != 0 || res == NULL) {
        LOG_ERR("getaddrinfo failed for host: " << host);
        return -1;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    std::memcpy(&addr, res->ai_addr, sizeof(addr));
    addr.sin_port = htons(port);
    freeaddrinfo(res);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        LOG_ERR("socket failed. errno = " << errno);
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        LOG_ERR("connect failed. errno = " << errno);
        return -1;
    }

    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        LOG_ERR("fcntl O_NONBLOCK failed. errno = " << errno);
        return -1;
    }

    LOG_INF("Connected to " << host << ":" << port);
    return 0;
}

int Bot::sendLine(const std::string &line) {
    write_buf.append(line);
    write_buf.append("\r\n");
    return 0;
}

int Bot::flushWrite(void) {
    if (write_buf.empty())
        return 0;
    ssize_t n = send(fd, write_buf.c_str(), write_buf.size(), MSG_NOSIGNAL);
    if (n > 0) {
        write_buf.erase(0, static_cast<size_t>(n));
        return 0;
    }
    LOG_ERR("send failed. errno = " << errno);
    return -1;
}

int Bot::registerWithServer(void) {
    LOG_INF("Registering as " << nick << ", joining " << channel);
    sendLine("PASS " + password);
    sendLine("NICK " + nick);
    sendLine("USER " + nick + " 0 * :IRC Bot");
    sendLine("JOIN " + channel);
    return flushWrite();
}

int Bot::receiveData(void) {
    char buf[BOT_RECV_CHUNK];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n > 0) {
        read_buf.append(buf, n);
        return 0;
    }
    if (n == 0) {
        LOG_WRN("Server closed connection");
        return -1;
    }
    LOG_ERR("recv failed. errno = " << errno);
    return -1;
}

void Bot::processMessages(void) {
    while (true) {
        IrcMessage msg;
        TokenizeResult r = tokenizeNextMessage(read_buf, msg);
        if (r == TOKENIZE_NO_FRAME)
            break;
        if (r == TOKENIZE_OK)
            handleMessage(msg);
        // TOO_LONG / EMPTY / MALFORMED: silently consumed; nothing to do.
    }
}

void Bot::handleMessage(const IrcMessage &msg) {
    LOG_DBG("Frame received: " << msg.getCommand());
    if (msg.getCommand() == "PING") {
        std::string token;
        if (msg.hasTrailing() && !msg.getParams().empty())
            token = msg.getParams().back();
        handlePing(token);
    } else if (msg.getCommand() == "PRIVMSG" && msg.getParams().size() >= 2) {
        std::string sender = nickFromPrefix(msg.getPrefix());
        handlePrivmsg(sender, msg.getParams()[0], msg.getParams()[1]);
    }
}

void Bot::handlePing(const std::string &trailing) {
    LOG_DBG("PING from server, replying PONG :" << trailing);
    sendLine("PONG :" + trailing);
}

void Bot::handlePrivmsg(const std::string &sender_nick,
                        const std::string &target,
                        const std::string &message) {
    // Reply to the channel if the message went to a channel; otherwise PM the sender.
    std::string reply_target;
    if (!target.empty() && (target[0] == '#' || target[0] == '&'))
        reply_target = target;
    else
        reply_target = sender_nick;

    if (message.empty() || message[0] != '!')
        return;

    std::string::size_type space = message.find(' ');
    std::string cmd;
    std::string args;
    if (space == std::string::npos) {
        cmd = message;
    } else {
        cmd = message.substr(0, space);
        args = message.substr(space + 1);
    }

    if (cmd == "!echo") {
        if (args.empty()) {
            LOG_DBG("!echo from " << sender_nick << " (target " << reply_target << "): empty args, ignoring");
            return;
        }
        LOG_INF("!echo from " << sender_nick << " (target " << reply_target << "): " << args);
        replyTo(reply_target, args);
    } else if (cmd == "!site") {
        LOG_INF("!site from " << sender_nick << " (target " << reply_target << ")");
        replyTo(reply_target, BOT_SITE_URL);
    } else {
        LOG_DBG("Unknown command from " << sender_nick << ": " << cmd);
    }
}

void Bot::replyTo(const std::string &reply_target, const std::string &msg) {
    LOG_DBG("Reply to " << reply_target << ": " << msg);
    sendLine("PRIVMSG " + reply_target + " :" + msg);
}

int Bot::run(volatile sig_atomic_t *shutdown_flag_) {
    this->shutdown_flag = shutdown_flag_;

    if (connectToServer() < 0)
        return 1;
    if (registerWithServer() < 0)
        return 1;

    while (!*(this->shutdown_flag)) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        if (!write_buf.empty())
            pfd.events |= POLLOUT;
        pfd.revents = 0;

        // 1s timeout so the loop also re-checks the shutdown flag periodically.
        int n = poll(&pfd, 1, 1000);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            LOG_ERR("poll failed. errno = " << errno);
            break;
        }
        if (n == 0)
            continue;

        if (pfd.revents & POLLIN) {
            if (receiveData() < 0)
                break;
            processMessages();
        }
        if (pfd.revents & POLLOUT) {
            if (flushWrite() < 0)
                break;
        }
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
            break;
    }

    // Best-effort QUIT before closing.
    sendLine("QUIT :bye");
    flushWrite();
    return 0;
}
