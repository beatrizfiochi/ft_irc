#include "Command.hpp"
#include "../log.hpp"
#include <vector>
#include "../utils/utils.hpp"
#include "../Server.hpp"

LOG_REGISTER(Command_handler_JOIN);

struct JoinTarget {
    std::string channel;
    std::string key;
};

// Examples from RFC:
//   JOIN #foobar                    ; join channel #foobar.
//   JOIN &foo fubar                 ; join channel &foo using key "fubar".
//   JOIN #foo,&bar fubar            ; join channel #foo using key "fubar"
//                                   and &bar using no key.
//   JOIN #foo,#bar fubar,foobar     ; join channel #foo using key "fubar".
//                                   and channel #bar using key "foobar".
//   JOIN #foo,#bar                  ; join channels #foo and #bar.
//   :WiZ JOIN #Twilight_zone        ; JOIN message from WiZ
static std::vector<JoinTarget> parseChannelsAndKeys(const std::vector<std::string>& args) {
    std::vector<JoinTarget> targets;

    std::vector<std::string> channels = split(args[0], ',');
    std::vector<std::string> keys;
    if (args.size() > 1)
        keys = split(args[1], ',');

    for (size_t i = 0; i < channels.size(); i++) {
        std::string ch = channels[i];

        if (ch.empty())
            continue;

        std::string key = "";
        if (i < keys.size())
            key = keys[i];

        JoinTarget target;
        target.channel = ch;
        target.key = key;

        targets.push_back(target);
    }

    return targets;
}

static bool isValidChannelName(const std::string &name)
{
    if (name.size() < 2 || name.size() > 200)
        return false;

    if (name[0] != '#' && name[0] != '&')
        return false;

    for (size_t i = 0; i < name.size(); i++)
    {
        char c = name[i];
        if (c == ' ')
            return false;
        if (c == ',')
            return false;
        if (c == '\a')
            return false;
    }

    return true;
}

// verificar se o canal existe -> se nao existe cria e o user 'e o operador
// se existir -> verifica restricoes (invite-only, key, limit
// (limite de pessoas no canal e limite de canais para aquele client))
// se passar pelas restricoes -> adiciona ao canal
// broadcast da mensagem
int Command::handleJoin(Server &server, Client &client)
{
    if (this->param.size() < 1)
        return server.sendReply(client.getFd(), ERR_NEEDMOREPARAMS,
                            this->getCmd(), "Not enough parameters");

    if (!client.isRegistered())
        return server.sendReply(client.getFd(), ERR_NOTREGISTERED,
                            "", "You have not registered");

    std::vector<JoinTarget> targets = parseChannelsAndKeys(this->param);

    for (size_t i = 0; i < targets.size(); i++) {
        std::string channelName = targets[i].channel;
        std::string key         = targets[i].key;

        if (!isValidChannelName(channelName)) {
            server.sendReply(client.getFd(), ERR_NOSUCHCHANNEL,
                            channelName, "No such channel");
            continue;
        }

        Channel* ch = server.getChannel(channelName);
        if (ch == NULL) {
            ch = &server.createChannel(channelName);

            ch->addClient(client.getFd());
            server.broadcastMsg(*ch, client, "JOIN " + channelName, "");
            // sendTopic(server, client, *ch);
            // sendNames(server, client, *ch);

            continue;
        }

    //     // =========================================================
    //     // CASO 2: canal já existe → validar regras
    //     // =========================================================

    //     if (ch->isUser(client))
    //         continue;

    //     if (ch->isInviteOnly() && !ch->isInvited(client))
    //     {
    //         server.sendReply(client.getFd(), ERR_INVITEONLYCHAN,
    //                         channelName, "Cannot join channel (+i)");
    //         continue;
    //     }

    //     if (ch->hasKey() && ch->getKey() != key)
    //     {
    //         server.sendReply(client.getFd(), ERR_BADCHANNELKEY,
    //                         channelName, "Bad channel key");
    //         continue;
    //     }

    //     if (ch->isFull())
    //     {
    //         server.sendReply(client.getFd(), ERR_CHANNELISFULL,
    //                         channelName, "Channel is full");
    //         continue;
    //     }

    //     // 3.3 adicionar cliente
    //     ch->addUser(&client);

    //     // 3.4 broadcast + info
    //     broadcastJoin(server, client, *ch);
    //     sendTopic(server, client, *ch);
    //     sendNames(server, client, *ch);
    }

    return 0;
}
