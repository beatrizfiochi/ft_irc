#include "Command.hpp"
#include "../log.hpp"

// TODO entender o funcionamento do Log
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
    // ERR_NOTONCHANNEL
    if (target == NULL) {
        LOG_DBG("No such nick/channel");
        return server.sendReply(target->getFd(), ERR_NOSUCHNICK, this->command, "No such nick/channel");
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
    server.broadcastMsg(*channel, client, "KICK " + channel->getName(),
    reason, client.getFd());
    server.kickClient(target->getFd(), *channel);
    return 0;
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
    Client *target = server.getClient(param[0]);
    if (!target) {
        LOG_DBG("No such nick/channel");
        return server.sendReply(client.getFd(), ERR_NOSUCHNICK, this->command, "No such nick/channel");
    }

    // There is no requirement that the channel the target user is being invited to
    // must exist or be a valid channel ( o RFC nao diz o que fazer nesse caso)
    // ERR_NOSUCHCHANNEL (coloquei para evitar segfault do adding do client ao channel = NULL)
    Channel *channel = server.getChannel(param[1]);
    if (channel == NULL) {
        LOG_DBG("No such channel");
        return server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL, param[1], "No such channel");
    }
    // ERR_CHANOPRIVSNEEDED
    if (channel->isInviteOnly() && !channel->isOperator(client.getFd())) {
        LOG_DBG("It needs a channel operator");
        return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, param[1], "It needs a channel operator");
    }

    channel->inviteClient(target->getFd());
    //channel->addClient(target->getFd());
    LOG_DBG("Adding User " + param[0] + " to channel " + param[1]);
    server.sendReply(client.getFd(), RPL_INVITING, param[0] + " " + channel->getName(), "");
    server.sendGenericMsg(client, *target, "INVITE " + target->getNick() + " " + channel->getName(), "");
    return 0;
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
        server.sendReply(client.getFd(), RPL_TOPIC, channel->getName(), channel->getTopic());
        return 0;
    }
    // topic for channel will be changed
    channel->setTopic(param[1]);
    server.broadcastMsg(*channel, client, "TOPIC " + channel->getName(), channel->getTopic(), client.getFd());
    return 0;
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

int Command::handleMode(Server &server, Client &client) {
    if (param.size() < 2) {
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }
    Channel *channel = server.getChannel(param[0]);
    if (channel == NULL){
        LOG_DBG("No such channel: " + param[0]);
        return server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL, param[0], "No such channel");
    }

    if (param[1].empty()) {
        LOG_DBG("Unknown MODE flag");
        return server.sendReply(client.getFd(), ERR_UMODEUNKNOWNFLAG, this->command, "Unknown MODE flag" );
    }

    //  Aqui eu tenho uma duvida conceitual: verifico toda a linha antes?
    std::string modes = param[1];
    size_t argIndex = 2;
    char sign = '+';
    for (size_t i = 0; i < modes.size(); i++) {
        char c = modes[i];
        if (c == '+' || c == '-') {
            sign = c;
            continue;
        }

        std::string arg;
        if (c == 'o' || ((c == 'k' || c == 'l') && sign == '+')) {
            if (argIndex >= param.size()) {
                return server.sendReply(client.getFd(), ERR_UMODEUNKNOWNFLAG, this->command, "Unknown MODE flag" );
            }
            arg = param[argIndex++];
        }

        // TODO implementar as funcoes de enable e disable / add e remove
        // if (c == 'i') {
        //     if (sign == '+') enableInviteOnly();
        //     else disableInviteOnly();
        // }
        // else if (c == 't') {
        //     if (sign == '+') enableTopicRestricted();
        //     else disableTopicRestricted();
        // }
        // else if (c == 'o') {
        //     if (sign == '+') addOperator(arg);
        //     else removeOperator(arg);
        // }
        // else if (c == 'k') {
        //     if (sign == '+') setKey(arg);
        //     else removeKey();
        // }
        // else if (c == 'l') {
        //     if (sign == '+') setLimit(std::atoi(arg.c_str()));
        //     else removeLimit();
        // }
        // else {
        //     std::cerr << "Modo inválido: " << c << std::endl;
        // }
    }
    return 0;
}
