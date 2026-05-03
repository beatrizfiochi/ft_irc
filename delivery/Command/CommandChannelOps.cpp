#include "Command.hpp"
#include "../log.hpp"

// TODO entender o funcionamento do Log
LOG_REGISTER(Command_ChannelOps);

int Command::handleKick(Server& server, Client& client) {
    if (param.size() < 2){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }
    // TODO: Verificar a existencia do canal (implementar no server)
    Channel *channel = server.getChannel(param[0]);
    if (channel == NULL){
       LOG_DBG("No such channel: " + param[0]);
       return server.sendReply  (client.getFd(),
                                 ERR_NOSUCHCHANNEL,
                                 param[0],
                                 "No such channel");
    }

    // TODO verificar a existencia do cliente (implementar no channel)
    Client* target = server.getClient(param[1]);
    if (!target) {
        return server.sendReply(client.getFd(), ERR_USERNOTINCHANNEL, param[1], "They aren't on that channel");
    }

    //TODO Verificar se cliente eh operador do canal (implementar no channel)
    if (!channel->isOperator(client.getFd())){
        LOG_DBG("User is not channel operator");
        return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, param[0], "You're not channel operator");
    }

    //  TODO o metodo no servidor que kicka o client. Ver ser o canal vai ficar vazio e/ou
    //  se o canal nao vai ter mais operador (implementar no server)
    //  if (param.size() > 2)
    //      return server.kickClient(client, param[0], param[1], param[2]);
    //  else
    //      return server.kickClient(client, param[0], param[1], "");
    return 0;
}

int Command::handleInvite(Server& server, Client& client) {

    if (param.size() < 2){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    // TODO implementar no servidor o getChannel os metodos bool de InviteOnly e isOperator no Channel
    Channel *channel = server.getChannel(param[0]);
    if (channel && channel->isInviteOnly() && !channel->isOperator(client.getFd())) {
        LOG_DBG("It needs a channel operator");
        return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, param[0], "It needs a channel operator");
    }

    // TODO implementar no servidor o getClient e o inviteClientToChannel
    Client *target = server.getClient(param[1]);
    (void) target;
    //server.inviteClientToChannel(target, param[0]);

    // envia INVITE pro target (essa linha esta incorreta)
    // server.sendMessage(*target, ":" + client.getNick() + " INVITE " + targetNick + " " + channelName);
    // server.sendMessage(client, "341 " + client.getNick() + " " + param[1] + " " + param[0]);
    return 0;
}

//    A channel operator is identified by the '@' symbol next to their
//    nickname whenever it is associated with a channel

//    Command: TOPIC
//    Parameters: <channel> [<topic>]

int Command::handleTopic(Server &server, Client &client) {
    if (param.size() < 1){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }
    Channel *channel = server.getChannel(param[0]);
    if (channel == NULL) {
        LOG_DBG("No such channel: " + param[0]);
        return server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL, param[0], "No such channel");
    }
    if (param.size() == 1) {
        if (channel->getTopic().empty())
            return server.sendReply(client.getFd(), RPL_NOTOPIC, this->command, "No topic is set");
        return server.sendReply(client.getFd(), RPL_TOPIC, channel->getName(), channel->getTopic());
    }
    // TODO
    if (channel->isTopicRestricted() && !channel->isOperator(client.getFd())) {
        LOG_DBG("It needs a channel operator");
        return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, this->command, "It needs a channel operator");
    }
    // topic for channel will be changed
    channel->setTopic(param[1]);
    return server.sendReply(client.getFd(), RPL_TOPIC, channel->getName(), channel->getTopic());
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
