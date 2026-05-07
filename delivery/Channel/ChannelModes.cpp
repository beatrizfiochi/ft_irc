#include "Channel.hpp"

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


// TODO verificar o >0 e <MAX_INT
void Channel::setLimit(long value) {
    userLimit = static_cast<int>(value);
}

void Channel::removeLimit() {
    this->userLimit = -1;
}

const std::string Channel::getModeList(void) const {
    return this->modeList;
}
