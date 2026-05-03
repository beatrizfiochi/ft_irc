#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <string>
#include <vector>
#include "../Client/Client.hpp"
#include "../Server.hpp"

// ERROR MESSAGES
#define ERR_NOSUCHNICK          401 // "<nickname> :No such nick/channel"
#define ERR_NOTEXTTOSEND        412 // ":No text to send"
#define ERR_NEEDMOREPARAMS      461 // "<command> :Not enough parameters"
#define ERR_ALREADYREGISTRED    462 // ":You may not reregister"
#define ERR_PASSWDMISMATCH      464 // ":Password incorrect"
#define ERR_NICKCOLLISION       436 // "<nick> :Nickname collision KILL"
#define ERR_ERRONEUSNICKNAME    432 // "<nick> :Erroneus nickname"
#define ERR_NONICKNAMEGIVEN     431 // ":No nickname given"
#define ERR_NOTREGISTERED       451 // ":You have not registered"
#define ERR_NOSUCHCHANNEL       403 // "<channel name> :No such channel"
#define ERR_CHANOPRIVSNEEDED    482 // "<channel> :You're not channel operator"
#define ERR_USERNOTINCHANNEL    441 // "<nick> <channel> :They aren't on that channel"
#define ERR_UMODEUNKNOWNFLAG    501 // ":Unknown MODE flag"
#define ERR_UNKNOWNMODE         472 // "<char> :is unknown mode char to me"
#define ERR_NOTDEFINED          999
#define ERR_INVITEONLYCHAN      473 // "<channel> :Cannot join channel (+i)"
#define ERR_BADCHANNELKEY       475 // "<channel> :Cannot join channel (+k)"
#define ERR_CHANNELISFULL       471 // "<channel> :Cannot join channel (+l)"
#define ERR_CANNOTSENDTOCHAN    404 // "<channel name> :Cannot send to channel"

#define RPL_MOTDSTART   375 // ":- <server> Message of the day - "
#define RPL_MOTD        372 // ":- <text>"
#define RPL_ENDOFMOTD   376 // ":End of /MOTD command"
#define RPL_INVITING    341 // "<channel> <nick>"
#define RPL_NOTOPIC     331 // "<channel> :No topic is set"
#define RPL_TOPIC       332 // "<channel> :<topic>"

// 324     RPL_CHANNELMODEIS "<channel> <mode> <mode params>" ??????

class Command {
public:
    static Command *parsing(const std::string &raw);
    const std::string& getCmd(void) const;
    const std::vector<std::string>& getParams(void) const;
    int execute(Server &server, Client &client);
private:
    std::string command;
    std::vector<std::string> param;

    Command(const std::string &command, const std::vector<std::string> &param);
    static size_t skipSpaces(const std::string &str, size_t pos);
    static std::string getNextToken(const std::string &str, size_t &pos);
    static bool isValidCommand(const std::string &cmd);
    static bool isValidParam(const std::string &param);
    bool isValidNick(const std::string &nickname);

    int handleUnknownCommand(Server &server, Client &client);
    int handlePass(Server &server, Client &client);
    int handleNick(Server &server, Client &client);
    int handleUser(Server &server, Client &client);
    int handleMOTD(Server &server, Client &client);
    int handleQuit(const std::vector<std::string> &args);
    int handlePrivMsg(Server &server, Client &client);
    int handleKick(Server &server, Client &client);
    int handleInvite(Server &server, Client &client);
    int handleTopic(Server &server, Client &client);
    int handleMode(Server &server, Client &client);
    int handleJoin(Server &server, Client &client);
};

#endif // _COMMAND_HPP_
