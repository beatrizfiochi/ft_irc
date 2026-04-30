#include <cctype>
#include "Client.hpp"
#include "../log.hpp"

LOG_REGISTER(client);

Client::Client() : fd(-1), nickName(""), userName(""), realName(""),
              passOk(false), registered(false) {}

Client::Client(int fd) : fd(fd), nickName(""), userName(""), realName(""),
              passOk(false), registered(false) {
    LOG_INF("Client " << fd << " created");
}

Client::Client(Client const &other) : fd(other.fd), nickName(other.nickName), userName(other.userName),
                realName(other.realName), passOk(other.passOk), registered(other.registered) {}

Client& Client::operator=(Client const &rhs) {
    if (this != &rhs)
    {
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

// TODOs
//         void getUser();
//         void getReal();
//
//         void setUser();
//         void setReal();
//
//         void setPassOK("true");
// 	       void setRegister("true");
//
//         bool isRegistered())
