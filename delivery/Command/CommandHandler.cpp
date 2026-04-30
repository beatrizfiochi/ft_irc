#include "Command.hpp"
#include "../log.hpp"

LOG_REGISTER(Command_handler);

int Command::handleUnknownCommand(Server &server, Client &client) {
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

    client.setNick(newNick);

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
    if (client.getPassFlag())
        client.setRegister(true);

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
// TODO
// A sugestao eh que o Server execute esse comando
// reason seria uma mensagem montada, tipo
// std::string reason = "Tchau ..."
// if (!args[0].empty())
//      reason = args[0];
// std::string msg = ":" + client.getNick() + " QUIT :" + reason;
// channel.broadcast(msg);
    return (0);
}

int Command::handlePrivMsg(Server &server, Client &client,
                                  const std::vector<std::string> &args) {
    (void)client;
    (void)server;

    if (args.size() < 2)
        return ERR_NEEDMOREPARAMS;

    // TODO check for client registred
    // if (!Client.isRegistered())
    //     return ERR_NOTREGISTERED;

    std::string message = args[1];

    if (message.empty())
        return ERR_NOTEXTTOSEND;

//TODOs implement split function
//      implement Server.getClientByNick
//      implement Server.sendError
//      implement Server.sendMessage
//
// std::vector<std::string> targets = split(args[0], ',');

// for (size_t i = 0; i < targets.size(); ++i) {

//     Client *receiver = Server.getClientByNick(targets[i]);

//     if (!receiver) {
//         Server.sendError(client, 401, targets[i]);
//         continue;
//     }

//     std::string msg = ":" + Client.getNick() +
//                       " PRIVMSG " + targets[i] + " :" + message;

//     Server.sendMessage(*receiver, msg);
// }
    return 0;
}
