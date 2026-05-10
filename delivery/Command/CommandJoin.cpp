#include "Command.hpp"
#include "../log.hpp"
#include <vector>
#include "../utils/utils.hpp"
#include "../Server.hpp"

LOG_REGISTER(Command_handler_JOIN);

struct JoinTarget {
    std::string channel;
    std::string key;
};

// Examples from RFC:
//   JOIN #foobar                    ; join channel #foobar.
//   JOIN &foo fubar                 ; join channel &foo using key "fubar".
//   JOIN #foo,&bar fubar            ; join channel #foo using key "fubar"
//                                   and &bar using no key.
//   JOIN #foo,#bar fubar,foobar     ; join channel #foo using key "fubar".
//                                   and channel #bar using key "foobar".
//   JOIN #foo,#bar                  ; join channels #foo and #bar.
//   :WiZ JOIN #Twilight_zone        ; JOIN message from WiZ
static std::vector<JoinTarget> parseChannelsAndKeys(const std::vector<std::string>& args) {
    std::vector<JoinTarget> targets;

    std::vector<std::string> channels = split(args[0], ',');
    std::vector<std::string> keys;
    if (args.size() > 1)
        keys = split(args[1], ',');

    for (size_t i = 0; i < channels.size(); i++) {
        std::string ch = channels[i];

        if (ch.empty())
            continue;

        std::string key = "";
        if (i < keys.size())
            key = keys[i];

        JoinTarget target;
        target.channel = ch;
        target.key = key;

        targets.push_back(target);
    }

    return targets;
}

static bool isValidChannelName(const std::string &name)
{
    if (name.size() < 2 || name.size() > 200)
        return false;

    if (name[0] != '#' && name[0] != '&')
        return false;

    for (size_t i = 0; i < name.size(); i++)
    {
        char c = name[i];
        if (c == ' ')
            return false;
        if (c == ',')
            return false;
        if (c == '\a')
            return false;
    }

    return true;
}

static int sendTopic(Server &server, Client &client, const Channel &channel) {
    const std::string channelName = channel.getName();
    const std::string topic = channel.getTopic();
    if (topic.empty())
        return server.sendReply(client.getFd(), RPL_NOTOPIC, channelName, "No topic is set");
    else
        return server.sendReply(client.getFd(), RPL_TOPIC, channelName, channel.getTopic());
}

static int sendNames(Server &server, Client &client, const Channel &channel) {
    const std::set<int> &members = channel.getMembers();

    std::string current;

    const size_t MAX_NAMES_LEN = 400;

    for (std::set<int>::const_iterator i = members.begin(); i != members.end(); ++i) {
        Client *member = server.getClientByFd(*i);
        if (member == NULL)
            continue;

        std::string nicks;

        if (channel.isOperator(*i))
            nicks += "@";
        nicks += member->getNick();

        size_t extra = nicks.size();
        if (!current.empty())
            extra += 1;

        // Handle max 512 bytes (IRC protocol)
        if (!current.empty() && current.size() + extra > MAX_NAMES_LEN) {
            if (server.sendReply(client.getFd(), RPL_NAMREPLY,
                    "= " + channel.getName(), current) < 0)
                return -1;
            current.clear();
        }

        if (!current.empty())
            current += " ";

        current += nicks;
    }

    if (!current.empty()) {
        if (server.sendReply(client.getFd(), RPL_NAMREPLY,
                    "= " + channel.getName(), current) < 0)
                return -1;
    }

    return server.sendReply(client.getFd(), RPL_ENDOFNAMES,
                    channel.getName(), "End of /NAMES list");
}

// Check if the channel exists -> if it doesn't exist, create the user and it's the operator;
// if it exists -> check restrictions (invite-only, key,
// limit (limit of people in the channel and limit of channels for that client));
// if it passes the restrictions -> add it to the message broadcast channel

int Command::handleJoin(Server &server, Client &client)
{
    if (this->param.size() < 1)
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS,
                            this->getCmd(), "Not enough parameters");

    if (!client.isRegistered())
        return server.sendReply(client.getFd(), ERR_NOTREGISTERED,
                            "", "You have not registered");

    std::vector<JoinTarget> targets = parseChannelsAndKeys(this->param);

    for (size_t i = 0; i < targets.size(); i++) {
        std::string channelName = targets[i].channel;
        std::string key         = targets[i].key;

        if (!isValidChannelName(channelName)) {
            server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL,
                            channelName, "No such channel");
            continue;
        }

        Channel* ch = server.getChannel(channelName);
        if (ch == NULL) {
            ch = &server.createChannel(channelName);
            if (!key.empty())
                 ch->setKey(key);
            ch->addClient(client.getFd());
            if (server.broadcastMsg(*ch, client, "JOIN " + channelName, "") < 0)
                return -1;
            if (sendTopic(server, client, *ch) < 0)
                return -1;
            if (sendNames(server, client, *ch) < 0)
                return -1;
            continue;
        }

        // Channel already exists
        // Check if it is already a member
        if (ch->isMember(client.getFd())) {
            // Silently ignore
            continue;
        }

        // TODO: Implement the isInvited
        if (ch->isInviteOnly() /* && !ch->isInvited(client) */) {
            server.sendReply(client.getFd(), ERR_INVITEONLYCHAN,
                            channelName, "Cannot join channel (+i)");
            continue;
        }

        if (ch->hasKey() && !ch->checkKey(key)) {
            server.sendReply(client.getFd(), ERR_BADCHANNELKEY,
                            channelName, "Cannot join channel (+k)");
            continue;
        }

        if (ch->isFull()) {
            server.sendReply(client.getFd(), ERR_CHANNELISFULL,
                            channelName, "Cannot join channel (+l)");
            continue;
        }

        // Add the user
        ch->addClient(client.getFd());
        if (server.broadcastMsg(*ch, client, "JOIN " + channelName, "") < 0)
            return -1;
        if (sendTopic(server, client, *ch) < 0)
            return -1;
        if (sendNames(server, client, *ch) < 0)
            return -1;
    }

    return 0;
}

int Command::handlePart(Server &server, Client &client) {
    if (this->param.size() < 1)
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command,
                            "Not enough parameters");

    if (!client.isRegistered())
        return server.sendReply(client.getFd(), ERR_NOTREGISTERED,
                            "", "You have not registered");

    std::vector<std::string> channels = split(this->param[0], ',');

    std::string reason = "";
    if (this->param.size() > 1)
        reason = this->param[1];

    for (size_t i = 0; i < channels.size(); i++) {
        std::string channelName = channels[i];

        if (channelName.empty())
            continue;

        Channel *ch = server.getChannel(channelName);
        if (ch == NULL) {
            server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL,
                            channelName, "No such channel");
            continue;
        }

        if (!ch->isMember(client.getFd())) {
            server.sendReply(client.getFd(), ERR_NOTONCHANNEL,
                            channelName, "You're not on that channel");
            continue;
        }

        if (server.broadcastMsg(*ch, client, "PART " + channelName, reason) < 0)
            return -1;

        ch->removeClient(client.getFd());

        if (ch->getMembers().empty())
            server.removeChannel(channelName);
    }

    return 0;

    }
