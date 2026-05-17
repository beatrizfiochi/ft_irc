/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandExecute.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bfiochi- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:15 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:15 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

    // Public: usable before registration. PASS/USER reject re-registration
    // internally; NICK works in both states; QUIT is allowed at any time.
    static const HandlerPair publicPairs[] = {
        HandlerPair("PASS", &Command::handlePass),
        HandlerPair("NICK", &Command::handleNick),
        HandlerPair("USER", &Command::handleUser),
        HandlerPair("QUIT", &Command::handleQuit),
    };
    static const std::map<std::string, CommandHandler> publicHandlers(
        publicPairs, publicPairs + sizeof(publicPairs) / sizeof(publicPairs[0]));

    // Private: dispatcher enforces ERR_NOTREGISTERED before reaching them.
    static const HandlerPair privatePairs[] = {
        HandlerPair("JOIN",    &Command::handleJoin),
        HandlerPair("PART",    &Command::handlePart),
        HandlerPair("PRIVMSG", &Command::handlePrivMsg),
        HandlerPair("PING",    &Command::handlePing),
        HandlerPair("KICK",    &Command::handleKick),
        HandlerPair("INVITE",  &Command::handleInvite),
        HandlerPair("TOPIC",   &Command::handleTopic),
        HandlerPair("MODE",    &Command::handleMode),
    };
    static const std::map<std::string, CommandHandler> privateHandlers(
        privatePairs, privatePairs + sizeof(privatePairs) / sizeof(privatePairs[0]));

    std::map<std::string, CommandHandler>::const_iterator it;

    it = publicHandlers.find(this->command);
    if (it != publicHandlers.end())
        return (this->*(it->second))(server, client);

    it = privateHandlers.find(this->command);
    if (it != privateHandlers.end()) {
        if (!client.isRegistered())
            return server.sendReply(client.getFd(), ERR_NOTREGISTERED,
                                    "", "You have not registered");
        return (this->*(it->second))(server, client);
    }

    return this->handleUnknownCommand(server, client);
}
