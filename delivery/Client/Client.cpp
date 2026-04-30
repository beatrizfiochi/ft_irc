#include <cctype>
#include "Client.hpp"

Client::Client() : fd(-1), nickName(""), userName(""), realName(""),
              passOk(false), registered(false) {}

Client::Client(int fd) : fd(fd), nickName(""), userName(""), realName(""),
              passOk(false), registered(false) {}

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

// TODOs
//         void getUser();
//         void getNick();
//         void getReal();
//
//         void setNick();
//         void setUser();
//         void setReal();
//
//         void setPassOK("true");
// 	       void setRegister("true");
//
//         bool isRegistered())
