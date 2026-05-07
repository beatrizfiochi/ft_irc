#include <cstdlib>
#include "Command.hpp"
#include "../log.hpp"
#include "../irc.hpp"
#include "../utils/utils.hpp"

LOG_REGISTER(Command_handler);

int Command::handleUnknownCommand(Server &server, Client &client) {
    LOG_ERR("Unknown command: " << this->getCmd());
    return server.sendReply(client.getFd(), 421, this->getCmd(), "Unknown command");
}

int Command::handlePass(Server &server, Client &client) {
    if (this->param.size() < 1) {
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS, this->command, "Not enough parameters");
    }

    if (client.isRegistered()) {
        LOG_DBG("User is already registered");
        return server.sendReply(client.getFd(), ERR_ALREADYREGISTRED, "", "You may not reregister");
    }

    if (!server.checkPass(this->param[0])) {
        LOG_DBG("Incorrect Password");
        return server.sendReply(client.getFd(), ERR_PASSWDMISMATCH, "", "Password incorrect");
    }

    LOG_DBG("Correct password");
    client.setPassFlag(true);

    return (0);
}

int Command::handleNick(Server &server, Client &client) {
    if (this->param.size() < 1)
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS,
                                this->command, "Not enough parameters");

    std::string newNick = this->param[0];
    if (!isValidNick(newNick))
        return server.sendReply(client.getFd(), ERR_ERRONEUSNICKNAME,
                                newNick, "Erroneus nickname");

    // TODO   **  Nicks list **
    //  if (server.nickExists(newNick))
    //      return ERR_NICKCOLLISION;

    std::string old = client.getNick();
    client.setNick(newNick);
    server.addClientToNickList(client, old);

    return (0);
}

// Command: USER
// Parameters: <username> <hostname> <servername> <realname>
int Command::handleUser(Server &server, Client &client) {
    if (this->param.size() < 4)
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS,
                                this->command, "Not enough parameters");

    if (client.isRegistered())
        return server.sendReply(client.getFd(), ERR_ALREADYREGISTRED,
                                this->command, "You may not reregister");

    // Setup User information
    // hostname and servername are ignored
    client.setUser(this->param[0]);
    client.setReal(this->param[3]);
    if (client.getPassFlag() && (client.getNick() != "*"))
        client.setRegister(true);
    else {
        LOG_ERR("Registration failed. Pass flag: " << client.getPassFlag() << ", Nick: " << client.getNick());
        server.disconnectClient(client.getFd());
        return EXIT_FAILURE;
    }

    return Command::handleMOTD(server, client);
}

int Command::handleMOTD(Server &server, Client &client) {
    int ret;

    ret = server.sendReply(client.getFd(), RPL_MOTDSTART,
                            "", "- " + server.getHostname() + " Message of the day -");
    if (ret > 0) {
        ret += server.sendReply(client.getFd(), RPL_MOTD,
                            "", "- Hello!! Funcionou?");
    }

    if (ret > 0) {
        ret += server.sendReply(client.getFd(), RPL_ENDOFMOTD,
                            "", "End of /MOTD command");
    }
    return ret;
}

int Command::handleQuit(const std::vector<std::string> &args) {
    (void)args;
// TODO tem de implementar QUIT
// A sugestao eh que o Server execute esse comando
// reason seria uma mensagem montada, tipo
// std::string reason = "Tchau ..."
// if (!args[0].empty())
//      reason = args[0];
// std::string msg = ":" + client.getNick() + " QUIT :" + reason;
// channel.broadcast(msg);
    return (0);
}

int Command::handlePrivMsg(Server &server, Client &client) {
    if (this->param.size() < 2)
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS,
                                this->command, "Not enough parameters");

    if (!client.isRegistered())
        return server.sendReply(client.getFd(), ERR_NOTREGISTERED,
                                this->command, "You have not registered");

    std::string &target_nick = this->param[0];
    std::string &message     = this->param[1];

    if (message.empty())
        return server.sendReply(client.getFd(), ERR_NOTEXTTOSEND,
                                "", "No text to send");

    std::vector<std::string> targets = split(target_nick, ',');
    LOG_DBG("Sending msg to " + target_nick);
    for (size_t i = 0; i < targets.size(); i++) {
        if (targets[i].empty())
            continue;
        // Check if it is a channel or a user
        if (isChannelName(targets[i])) {
            Channel *ch = server.getChannel(targets[i]);
            if (ch == NULL) {
                server.sendReply(client.getFd(), ERR_NOSUCHNICK,
                                        targets[i], "No such nick/channel");
                continue;
            }
            if (!ch->isMember(client.getFd())) {
                server.sendReply(client.getFd(), ERR_CANNOTSENDTOCHAN,
                                        targets[i], "Cannot send to channel");
                continue;
            }
            LOG_DBG("Msg to channel : " + targets[i] + ": " + message);
            server.broadcastMsg(*ch, client, "PRIVMSG " + ch->getName(),
                                message, client.getFd());
        } else {
            Client *target_client = server.getClient(targets[i]);
            if (target_client == NULL){
                LOG_DBG("Target not found: " + targets[i]);
                server.sendReply(client.getFd(), ERR_NOSUCHNICK,
                                        targets[i], "No such nick/channel");
                continue;
            }
            LOG_DBG("Msg to : " + targets[i] + ": " + message);
            int ret = server.sendPrivMsg(client, *target_client, message);
            if (ret < 0) {
                LOG_ERR("Sending msg on PRIVMSG returned error (" << ret << ")");
                return ret;
            }
        }
    }
    return 0;
}
