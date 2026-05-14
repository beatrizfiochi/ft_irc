#include "Channel.hpp"
#include "../Client/Client.hpp"
#include <sstream>

void Channel::enableInviteOnly() {
    this->setInviteOnly(true);
}

void Channel::disableInviteOnly() {
    this->setInviteOnly(false);
}

void Channel::enableTopicRestricted() {
    this->topicRestricted = true;
}

void Channel::disableTopicRestricted() {
    this->topicRestricted = false;
}

void Channel::addOperator(int fd) {
    operators.insert(fd);
}

void Channel::removeOperator(int fd) {
    operators.erase(fd);
}

void Channel::setPass(const std::string &key) {
    this->setKey(key);
}

void Channel::removePass() {
    this->setKey("");
}

void Channel::setLimit(long value) {
    userLimit = static_cast<int>(value);
}

void Channel::removeLimit() {
    this->userLimit = -1;
}

std::string Channel::getModeList(void) const {
    std::string modes;
    std::string args;
    if (inviteOnly)
        modes += 'i';
    if (topicRestricted)
        modes += 't';
    if (!key.empty()) {
        modes += 'k';
        args += " " + key;
    }
    if (userLimit != -1) {
        std::stringstream ss;
        ss << userLimit;
        modes += 'l';
        args += " " + ss.str();
    }
    return modes + args;
}
