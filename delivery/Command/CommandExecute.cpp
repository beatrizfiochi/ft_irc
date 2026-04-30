#include "Command.hpp"
#include "../log.hpp"
#include "../Server.hpp"
#include "../Client/Client.hpp"

LOG_REGISTER(CmdExecute);

int Command::execute(Server &server, Client &client) {
    (void)server;
    (void)client;
    int ret = 0;

    if (this->command == "NICK")
        LOG_DBG("NICK Command received");
    else if (this->command == "PASS")
        ret = this->handlePass(server, client);
    else
        ret = this->handleUnknownCommand(server, client);

    return ret;
}
