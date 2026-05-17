/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bfiochi- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:16 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:16 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <cstddef>
#include "Client.hpp"
#include "../log.hpp"

LOG_REGISTER(client);

Client::Client() : sessionId(0), fd(-1), nickName("*"), userName(""), realName(""),
              passOk(false), registered(false) {}

Client::Client(int fd, std::size_t sessionId) : sessionId(sessionId), fd(fd), nickName("*"), userName(""), realName(""),
              passOk(false), registered(false) {
    LOG_INF("Client " << fd << " created");
}

Client::Client(int fd, const std::string &nickname, std::size_t sessionId) : sessionId(sessionId), fd(fd), nickName(nickname), userName(""), realName(""),
              passOk(false), registered(false) {
    LOG_INF("Client " << fd << ", " << nickname << " created");
}

Client::Client(Client const &other) : sessionId(other.sessionId), fd(other.fd), nickName(other.nickName), userName(other.userName),
                realName(other.realName), passOk(other.passOk), registered(other.registered) {}

Client& Client::operator=(Client const &rhs) {
    if (this != &rhs)
    {
        this->sessionId = rhs.sessionId;
        this->fd = rhs.fd;
        this->nickName = rhs.nickName;
        this->userName = rhs.userName;
        this->realName = rhs.realName;
        this->passOk = rhs.passOk;
        this->registered = rhs.registered;
    }
    return (*this);
}

Client::~Client(){}

std::string& Client::getReadBuf(void) {
    return this->read_buf;
}

std::string& Client::getWriteBuf(void) {
    return this->write_buf;
}

std::size_t Client::getSessionId(void) const {
    return this->sessionId;
}

int Client::getFd(void) const {
    return this->fd;
}

std::string Client::getNick(void) const {
    return this->nickName;
}

void Client::setNick(const std::string &nick) {
    LOG_INF("Client [" << this->fd << "]: Nick changed from \"" <<
            this->nickName << "\" to \"" << nick << "\"");
    this->nickName = nick;
}

void Client::setRegister(bool flag) {
    this->registered = flag;
}

bool Client::isRegistered(void) const {
    return this->registered;
}

bool Client::getPassFlag(void) {
    return this->passOk;
}

void Client::setPassFlag(bool flag) {
    this->passOk = flag;
}

std::string Client::getUser(void) const {
    return this->userName;
}

void Client::setUser(const std::string &username) {
    this->userName = username;
}

std::string Client::getReal(void) const {
    return this->realName;
}

void Client::setReal(const std::string &realname) {
    this->realName = realname;
}
