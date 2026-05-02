#include "Channel.hpp"

Channel::Channel(void)
        : name(""), topic(""),
        inviteOnly(false),
        topicRestricted(false),
        key(""), userLimit(-1) {}

Channel::Channel(const std::string &channelName)
        : name(channelName),
        topic(""), inviteOnly(false),
        topicRestricted(false),
        key(""), userLimit(-1) {}

Channel::Channel(const Channel& other)
        : name(other.name), topic(other.topic),
        members(other.members), operators(other.operators),
        inviteOnly(other.inviteOnly),
        topicRestricted(other.topicRestricted),
        key(other.key), userLimit(other.userLimit) {}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        this->name = other.name;
        this->topic = other.topic;
        this->members = other.members;
        this->operators = other.operators;
        this->inviteOnly = other.inviteOnly;
        this->topicRestricted = other.topicRestricted;
        this->key = other.key;
        this->userLimit = other.userLimit;
    }
    return *this;
}

Channel::~Channel(void) {}

const std::string Channel::getName(void) const {
    return this->name;
}

void Channel::addClient(int fd) {
    if(members.insert(fd).second)
    {
        if(members.size() == 1)
            operators.insert(fd);
    }
}

void Channel::removeClient(int fd) {
    operators.erase(fd);
    members.erase(fd);
}

bool Channel::isMember(int fd) const {
    return members.find(fd) != members.end();
}

bool Channel::isOperator(int fd) const {
    return operators.find(fd) != operators.end();
}

void Channel::setTopic(const std::string& topic) {
    this->topic = topic;
}

const std::string Channel::getTopic(void) const {
    return this->topic;
}

const std::set<int>& Channel::getMembers() const
{
    return members;
}
