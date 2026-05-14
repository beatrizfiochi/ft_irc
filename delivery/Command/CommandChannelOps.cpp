#include "Command.hpp"
#include "../Channel/Channel.hpp"
#include "../log.hpp"
#include <cstdlib>

LOG_REGISTER(Command_ChannelOps);

//       Command: KICK
//    Parameters: <channel> <user> [<comment>]

int Command::handleKick(Server& server, Client& client) {
    // ERR_NEEDMOREPARAMS
    if (param.size() < 2){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    // ERR_NOSUCHCHANNEL
    Channel *channel = server.getChannel(param[0]);
    if (channel == NULL){
       LOG_DBG("No such channel: " + param[0]);
       return server.sendReply  (client.getFd(),
                                 ERR_NOSUCHCHANNEL,
                                 param[0],
                                 "No such channel");
    }

    Client *target = server.getClient(param[1]);
    // ERR_NOSUCHNICK
    if (target == NULL) {
        LOG_DBG("No such nick/channel");
        return server.sendReply(client.getFd(), ERR_NOSUCHNICK, param[1], "No such nick/channel");
    }

    // ERR_CHANOPRIVSNEEDED
    if (!channel->isOperator(client.getFd())){
        LOG_DBG("User is not channel operator");
        return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, param[0], "You're not channel operator");
    }

    // ERR_USERNOTINCHANNEL
    if (!channel->isMember(target->getFd())) {
        LOG_DBG("They aren't on that channel");
        return server.sendReply(client.getFd(), ERR_USERNOTINCHANNEL, param[1] + " " + param[0], "They aren't on that channel");
    }

    std::string reason = "";
    if (param.size() > 2)
        reason = param[2];
    LOG_DBG("Kicking User " + param[1] + " out of channel");
    // Capture target fd before broadcasting: the broadcast can disconnect
    // target (a channel member) on a write failure, leaving `target` dangling.
    int targetFd = target->getFd();
    int ret = server.broadcastMsg(*channel, client,
                                  "KICK " + channel->getName() + " " + param[1],
                                  reason);
    server.kickClient(targetFd, *channel);
    return ret;
}

//      Command: INVITE
//    Parameters: <nickname> <channel>

int Command::handleInvite(Server& server, Client& client) {
    // ERR_NEEDMOREPARAMS
    if (param.size() < 2){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    // ERR_NOSCHNICK
    const std::string invited_nickname = param[0];
    Client *target = server.getClient(invited_nickname);
    if (!target) {
        LOG_DBG("No such nick/channel");
        return server.sendReply(client.getFd(), ERR_NOSUCHNICK, this->command, "No such nick/channel");
    }

    const std::string ch_name = param[1];
    Channel *channel = server.getChannel(ch_name);
    // There is no requirement that the channel the target user is being invited to
    // must exist or be a valid channel
    if (channel != NULL) {
        if (!channel->isMember(client.getFd())) {
            LOG_DBG("User doing the invitation (" + client.getNick() + ") is NOT member of the channel " + ch_name);
            return server.sendReply(client.getFd(), ERR_NOTONCHANNEL,
                                    ch_name, "You're not on that channel");
        }
        // ERR_CHANOPRIVSNEEDED
        if (channel->isInviteOnly() && !channel->isOperator(client.getFd())) {
            LOG_DBG("It needs a channel operator");
            return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED,
                                    ch_name, "It needs a channel operator");
        }
        if (channel->isMember(target->getFd())) {
            LOG_DBG("User " + target->getNick() + " is already member of the channel " + ch_name);
            return server.sendReply(client.getFd(), ERR_USERONCHANNEL,
                                    target->getNick() + " " + channel->getName(),
                                    "is already on channel");
        }
        channel->inviteClient(target->getSessionId());
    }
    LOG_DBG("Inviting user " + invited_nickname + " to channel " + ch_name);
    if (server.sendReply(client.getFd(), RPL_INVITING, invited_nickname + " " + ch_name, "") < 0)
        return -1;
    return server.sendGenericMsg(client, *target, "INVITE " + target->getNick() + " " + ch_name, "");
}

//    Command: TOPIC
//    Parameters: <channel> [<topic>]

int Command::handleTopic(Server &server, Client &client) {
    // ERR_NEEDMOREPARAMS
    if (param.size() < 1){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    // ERR_NOSUCHCHANNEL
    Channel *channel = server.getChannel(param[0]);
    if (channel == NULL) {
        LOG_DBG("No such channel: " + param[0]);
        return server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL, param[0], "No such channel");
    }

    // ERR_NOTONCHANNEL
    if (!channel->isMember(client.getFd())) {
        LOG_DBG("They aren't on that channel");
        return server.sendReply(client.getFd(), ERR_NOTONCHANNEL, param[0], "They aren't on that channel");
    }

    // ERR_CHANOPRIVSNEEDED
    if (param.size() > 1) {
        if (channel->isTopicRestricted() && !channel->isOperator(client.getFd())) {
            LOG_DBG("It needs a channel operator");
            return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, param[0], "It needs a channel operator");
        }
    }

    if (param.size() == 1) {
        if (channel->getTopic().empty()) {
            LOG_DBG("No topic is set");
            return server.sendReply(client.getFd(), RPL_NOTOPIC, param[0], "No topic is set");
        }
        return server.sendReply(client.getFd(), RPL_TOPIC, channel->getName(), channel->getTopic());
    }
    // topic for channel will be changed
    channel->setTopic(param[1]);
    return server.broadcastMsg(*channel, client, "TOPIC " + channel->getName(), channel->getTopic());
}

//  Parameters: <channel> {[+|-]|o|p|s|i|t|n|b|v} [<limit>] [<user>]
//                [<ban mask>]

//    The MODE command is provided so that channel operators may change the
//    characteristics of `their' channel.  It is also required that servers
//    be able to change channel modes so that channel operators may be
//    created.

//    The various modes available for channels are as follows:

//            o - give/take channel operator privileges;
//            i - invite-only channel flag;
//            t - topic settable by channel operator only flag;
//            l - set the user limit to channel;
//            k - set a channel key (password).

//  When parsing MODE messages, it is recommended that the entire message
//  be parsed first and then the changes which resulted then passed on.

struct temporaryMode {
    char sign;
    char mode;
    std::string arg;
    long limitValue;

    temporaryMode(char s, char m, const std::string &a)
        : sign(s), mode(m), arg(a), limitValue(0) {}
};

int Command::handleMode(Server &server, Client &client) {
    if (param.size() < 1) {
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    Channel *channel = server.getChannel(param[0]);
    if (channel == NULL) {
        LOG_DBG("No such channel: " + param[0]);
        return server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL, param[0], "No such channel");
    }

    if (param.size() == 1) {
        LOG_DBG("Display MODEs");
        return server.sendReply(client.getFd(), RPL_CHANNELMODEIS, channel->getName() + " +" + channel->getModeList(), "");
    }

    if (!channel->isOperator(client.getFd())) {
        LOG_DBG("It needs a channel operator");
        return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, param[0], "It needs a channel operator");
    }

// -------------------------------------------------------------------------------------------------------------------

    std::string modes = param[1];
    size_t argIndex = 2;
    char sign = '+';
    std::vector<temporaryMode> changes;
    for (size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];
        if (c == '+' || c == '-'){
            sign = c;
            continue;
        }

        if (c != 'i' && c != 't' && c != 'o' && c != 'k' && c != 'l') {
            LOG_DBG("Unknown mode");
            return server.sendReply(client.getFd(), ERR_UNKNOWNMODE, std::string(1, c), "is unknown mode char to me");
        }

        // Modes that need arguments
        std::string arg;
        long value = 0;
        if (c == 'o' || ((c == 'k' || c == 'l') && sign == '+')) {
            if (argIndex >= param.size()) {
                LOG_DBG("Not enough parameters");
                return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
            }
            arg = param[argIndex++];
        }

        if (c == 'o') {
            Client *target = server.getClient(arg);
            // ERR_NOSUCHNICK
            if (target == NULL) {
                LOG_DBG("No such nick/channel");
                return server.sendReply(client.getFd(), ERR_NOSUCHNICK, arg, "No such nick/channel");
            }
            int targetFd = target->getFd();
            if (!channel->isMember(targetFd)) {
                LOG_DBG("User not in channel: " + arg);
                return server.sendReply(client.getFd(), ERR_USERNOTINCHANNEL, arg + " " + param[0], "They aren't on that channel");
            }
        }

        if (c == 'l' && sign == '+') {
            char* end;
            value = std::strtol(arg.c_str(), &end, 10);
            if ((*end != '\0') || value <= 0 || value > INT_MAX){
                LOG_DBG("Invalid limit string: " + arg);
                return server.sendReply(client.getFd(), ERR_UNKNOWNMODE, std::string(1, c) ,"Invalid limit string: " + arg);
            }
        }
        temporaryMode change(sign, c, arg);
        change.limitValue = value;
        changes.push_back(change);
    }

    // --------------------------------------------------------------------------------------------------------------------

    for (size_t i = 0; i < changes.size(); ++i)
    {
        temporaryMode &change = changes[i];

        switch (change.mode)
        {
            case 'i': {
                if (change.sign == '+')
                    channel->enableInviteOnly();
                else
                    channel->disableInviteOnly();
                break;
            }
            case 't': {
                if (change.sign == '+')
                    channel->enableTopicRestricted();
                else
                    channel->disableTopicRestricted();
                break;
            }
            case 'k': {
                if (change.sign == '+')
                    channel->setPass(change.arg);
                else
                    channel->removePass();
                break;
            }
            case 'o': {
                Client *target = server.getClient(change.arg);
                int targetFd = target->getFd();
                if (change.sign == '+') {
                    if (!channel->isOperator(targetFd)) {
                        channel->addOperator(targetFd);
                    }
                }
                else {
                    if (channel->isOperator(targetFd)) {
                        channel->removeOperator(targetFd);
                    }
                }
                break;
            }
            case 'l' : {
                if (change.sign == '+') {
                    channel->setLimit(change.limitValue);
                }
                else
                    channel->removeLimit();
                break;
            }
        }
    }

    std::string modeParameters = param[1];
    for (size_t i = 2; i < param.size(); i++)
        modeParameters += " " + param[i];
    return server.broadcastMsg(*channel, client, "MODE " + channel->getName(), modeParameters);
}
