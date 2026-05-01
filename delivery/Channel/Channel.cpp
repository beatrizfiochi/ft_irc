#include "Channel.hpp"
#include "../Client/Client.hpp"

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

void Channel::addClient(Client& client) {
    if(isMember(client))
        return;

    int clientFd = client.getFd();

    members[clientFd] = &client;

    if (members.size() == 1)
        operators[clientFd] = &client;
}

void Channel::removeClient(Client& client) {
    if(!isMember(client))
        return;

    int clientFd = client.getFd();

    if(isOperator(client))
        operators.erase(clientFd);

    members.erase(clientFd);
}

bool Channel::isMember(Client& client) const {
    return members.find(client.getFd()) != members.end();
}

bool Channel::isOperator(Client& client) const {
    return operators.find(client.getFd()) != operators.end();
}

void Channel::broadcast(const std::string& msg, Client *exclude) {
    for(std::map<int, Client*>::iterator i = members.begin(); i != members.end(); i++) {
        Client *client = i->second;

        if(exclude && client == exclude)
            continue;

        client->getWriteBuf() += msg + "\r\n";
    }
}

void Channel::setTopic(const std::string& topic) {
    this->topic = topic;
}

const std::string Channel::getTopic(void) const {
    return this->topic;
}
