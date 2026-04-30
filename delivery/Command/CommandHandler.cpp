#include "Command.hpp"


int Command::handlePass(Client &client, const std::vector<std::string> &args, const std::string &serverPassword) {
	(void)client;
	if (args.size() < 1)
		return (ERR_NEEDMOREPARAMS);

	if (args[0] != serverPassword)
        return (ERR_NOTDEFINED);
// TODO     SetPass
//	Client.setPassOK("true");
	return (0);
}

int Command::handleNick(Client &client, const std::vector<std::string> &args) {
	(void)client;
	if (args.size() < 1)
		return (ERR_NEEDMOREPARAMS);

    if (args[0].empty() || args[0].size() > 9)
		return (ERR_ERRONEUSNICKNAME);

    std::string newNick = args[0];
	if (!isValidNick(newNick))
		return (ERR_ERRONEUSNICKNAME);

// TODO   **  Nicks list **
//  if (Server.nickExists(newNick))
//  	return ERR_NICKCOLLISION;
// Se Client tiver Nick, remover o Nick do Server
//  if (!Client.getNick().empty())
//      Server.removeNick(Client.getNick())
//  Client.setNick(newNick);
//  Server.addNick(newNick, &client)

    return (0);
}

int Command::handleUser(Client &client, const std::vector<std::string> &args) {
	(void)client;
	if (args.size() < 4)
		return (ERR_NEEDMOREPARAMS);
// TODO   ** check if user is already registered
//      if (Client.isRegistered())
//          return (ERR_ALREADYREGISTERED);
// Setup User information
//      Client.setUser(args[0]);
//      Client.setReal(args[3]);
// 	    Client.setRegister(true);
	return (0);
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
