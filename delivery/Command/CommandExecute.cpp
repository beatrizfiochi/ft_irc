#include "Command.hpp"
#include "../log.hpp"
#include "../Server.hpp"
#include "../Client/Client.hpp"

LOG_REGISTER(CmdExecute);

int Command::execute(Server &server, Client &client) {
    (void)server;
    (void)client;
    int ret = 0;

    //TODO: Move this if/else to a map
    if (this->command == "NICK")
        ret = this->handleNick(server, client);
    else if (this->command == "PASS")
        ret = this->handlePass(server, client);
    else if (this->command == "USER")
        ret = this->handleUser(server, client);
    else if (this->command == "PRIVMSG")
        ret = this->handlePrivMsg(server, client);
    else if (this->command == "JOIN")
        ret = this->handleJoin(server, client);
    else if (this->command == "KICK")
        ret = this->handleKick(server, client);
    else if (this->command == "INVITE")
        ret = this->handleInvite(server, client);
    else
        ret = this->handleUnknownCommand(server, client);

    return ret;
}
