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
    else
        ret = this->handleUnknownCommand(server, client);

    return ret;
}
