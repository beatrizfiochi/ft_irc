#include "Command.hpp"
#include "../log.hpp"
#include "../Server.hpp"
#include "../Client/Client.hpp"
#include <map>
#include <utility>

LOG_REGISTER(CmdExecute);

int Command::execute(Server &server, Client &client) {
    typedef int (Command::*CommandHandler)(Server&, Client&);

    typedef std::pair<std::string, CommandHandler> HandlerPair;
    static const HandlerPair pairs[] = {
        HandlerPair("NICK", &Command::handleNick),
        HandlerPair("PASS", &Command::handlePass),
        HandlerPair("USER", &Command::handleUser),
        HandlerPair("PRIVMSG", &Command::handlePrivMsg),
        HandlerPair("JOIN", &Command::handleJoin),
        HandlerPair("KICK", &Command::handleKick),
        HandlerPair("INVITE", &Command::handleInvite),
        HandlerPair("TOPIC", &Command::handleTopic),
        HandlerPair("MODE", &Command::handleMode),
        HandlerPair("PING", &Command::handlePing),
        HandlerPair("PART", &Command::handlePart),
        HandlerPair("QUIT", &Command::handleQuit),
    };

    static const std::map<std::string, CommandHandler>
                handlers(pairs, pairs + sizeof(pairs) / sizeof(pairs[0]));

    std::map<std::string, CommandHandler>::const_iterator it = handlers.find(this->command);

    if (it != handlers.end())
        return (this->*(it->second))(server, client);

    return this->handleUnknownCommand(server, client);
}
