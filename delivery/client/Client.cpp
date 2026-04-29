/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmoura-p <cmoura-p@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 13:08:09 by cmoura-p          #+#    #+#             */
/*   Updated: 2026/04/29 14:32:46 by cmoura-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : fd(-1), nickName(""), userName(""), realName(""),
              passOk(false), registered(false) {}

Client::Client(Client const &other) : fd(other.fd), nickName(other.nickName), userName(other.userName),
                realName(other.realName), passOk(other.passOk), registered(other.registered) {}

Client& Client::operator=(Client const &rhs) {
    if (this != &rhs)
    {
        this->fd = rhs.fd;
        this->nickName = rhs.nickName;
        this->userName = rhs.userName;
        this->realName = rhs.realName;
        this->passOk = rhs.passOk;
        this->registered = rhs.registered;
    }
    return (*this);
}

Client::~Client(){}

int Client::handlePass(const std::vector<std::string> &args, const std::string &serverPassword) {
	if (args.size() < 1) {
		return (ERR_NEEDMOREPARAMS);
	}
// TODO     Check server.password
	if (args[0] != serverPassword)
		return (ERR_NOTDEFINED);
	this->passOk = true;
	return (0);
}

//int Client::handleNick(const std::vector<std::string> &args, Server &server) {
int Client::handleNick(const std::vector<std::string> &args) {
	if (args.size() < 1)
		return (ERR_NEEDMOREPARAMS);

	if (args[0].empty() || args[0].size() > 9)
		return (ERR_ERRONEUSNICKNAME);

	if (!isValidNick(args[0]))
		return (ERR_ERRONEUSNICKNAME);
	// TODO     Nicks list
	//if (server.nickExists(args[0]))
	//	return ERR_NICKCOLLISION;
	this->nickName = args[0];
	return (0);
}

int Client::handleUser(const std::vector<std::string> &args) {
	if (args.size() < 4)
		return (ERR_NEEDMOREPARAMS);
	if (this->registered)
		return (ERR_ALREADYREGISTERED);
	this->userName = args[1];
	this->realName = args[3];
	this->registered = true;
	return (0);
}

// <nick> = <letter> { <letter> | <number> | <special> }
// <letter> = 'a' ... 'z' | 'A' ... 'Z'
// <number> = '0' ... '9'
// <special> = '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'

bool Client::isValidNick(const std::string &nickname) {

	if (!std::isalpha(nickname[0]))
		return false;

	for (size_t i = 1; i < nickname.size(); ++i) {
		unsigned char c = (unsigned char)nickname[i];
		if (!std::isalnum(c) && c != '-' && c != '[' &&
		    c != ']' && c != '\\' && c != '^' && c != '_' &&
		    c != '{' && c != '}' && c != '|' && c != '`') {
			return false;
		}
	}
	return true;
}
