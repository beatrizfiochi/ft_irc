#include "Command.hpp"
#include "../log.hpp"

// TODO entender o funcionamento do Log
LOG_REGISTER(Command_ChannelOps);

int Command::handleKick(Server& server, Client& client) {
    (void) server;
    (void) client;
    if (param.size() < 2){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }
    // TODO: Verificar a existencia do canal (implementar no server)
    // Channel *channel = Server.getChannel(param[0]);
    // if (channel == NULL){
    //     LOG_DBG("No such channel: " + param[0]);
    //     return server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL, param[0], "No such channel");
    // }

    // TODO verificar a existencia do cliente (implementar no channel)
    // Client* target = channel->getMemberByNick(param[1]);
    // if (!target) {
    //     return server.sendReply(client.getFd(), ERR_USERNOTINCHANNEL, param[1], "They aren't on that channel");
    // }

    // TODO Verificar se cliente eh operador do canal (implementar no channel)
    // if (!channel->isOperator(client)){
    //     LOG_DBG("User is not channel operator");
    //     return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEED, param[0], "You're not channel operator");
    // }

    //  TODO o metodo no servidor que kicka o client. Ver ser o canal vai ficar vazio e/ou
    //  se o canal nao vai ter mais operador (implementar no server)
    //  if (param.size() > 2)
    //      return server.kickClient(client, param[0], param[1], param[2]);
    //  else
    //      return server.kickClient(client, param[0], param[1]);
    return 0;
}

int Command::handleInvite(Server& server, Client& client) {
    (void) server;
    (void) client;
    if (param.size() < 2){
        LOG_DBG("Not enough parameters");
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    // TODO implementar no servidor o getChannel os metodos bool de InviteOnly e isOperator no Channel
    // Channel *channel = Server.getChannel(param[0]);
    // if (channel && channel->isInviteOnly() && !channel->isOperator(client)) {
    //     LOG_DBG("It needs a channel operator");
    //     return server.sendReply(client.getFd(), ERR_CHANOPRIVSNEEDED, this->command, "It needs a channel operator");
    // }

    // TODO implementar no servidor o getClient e o inviteClientToChannel
    // Client *target = server.getClient(param[1]);
    // server.inviteClientToChannel(target, param[0]);

    // envia INVITE pro target (essa linha esta incorreta)
    // server.sendMessage(*target, ":" + client.getNick() + " INVITE " + targetNick + " " + channelName);
    // server.sendMessage(client, "341 " + client.getNick() + " " + param[1] + " " + param[0]);
    return 0;
}
