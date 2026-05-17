/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djunho <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:15 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:15 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <string>
#include <vector>
#include "Command.hpp"
#include "../log.hpp"
#include "../utils/IrcParse.hpp"

LOG_REGISTER(Command);

Command::Command(const std::string &command, const std::vector<std::string> &param) : command(command), param(param) {
    if (LOG_IS_DBG_ENABLED) {
        LOG_DBG("Created Command: ");
        LOG_DBG("\t command: " + this->command);
        for(size_t i = 0; i < this->param.size(); ++i){
            LOG_DBG("\t param: " + this->param[i]);
        }
    }
}

// From 2.3 Messages of the RFC 1459
//
//  Each IRC message may consist of up to three main parts: the prefix
//  (optional), the command, and the command parameters (of which there
//  may be up to 15).  The prefix, command, and all parameters are
//  separated by one (or more) ASCII space character(s) (0x20).
//
//  ...
//
//  IRC messages are always lines of characters terminated with a CR-LF
//  (Carriage Return - Line Feed) pair, and these messages shall not
//  exceed 512 characters in length, counting all characters including
//  the trailing CR-LF. Thus, there are 510 characters maximum allowed
//  for the command and its parameters.  There is no provision for
//  continuation message lines.  See section 7 for more details about
//  current implementations.

// Server-side validation on top of an already-tokenized message. Framing
// (CRLF + size cap) and grammar tokenization happen earlier in
// utils/IrcParse::tokenizeNextMessage. Here we only enforce server-specific
// rules: reject client-supplied prefixes (no server-to-server traffic),
// validate command grammar, and validate per-param char rules.
Command *Command::fromMessage(const IrcMessage &msg) {
    if (!msg.getPrefix().empty()) {
        LOG_ERR("Client-supplied prefix not supported");
        return NULL;
    }

    if (!Command::isValidCommand(msg.getCommand())) {
        LOG_ERR("Command (" + msg.getCommand() + ") not valid");
        return NULL;
    }

    // Per-param validation: middle params disallow space/nul/cr/lf and must be
    // non-empty; trailing only disallows nul/cr/lf and may be empty.
    const std::vector<std::string> &params = msg.getParams();
    for (size_t i = 0; i < params.size(); ++i) {
        bool isTrailing = (msg.hasTrailing() && i + 1 == params.size());
        const std::string &p = params[i];
        if (!isTrailing) {
            if (p.empty()) {
                LOG_ERR("Empty middle parameter");
                return NULL;
            }
            for (std::string::const_iterator it = p.begin(); it != p.end(); ++it) {
                if (*it == ' ' || *it == '\r' || *it == '\n' || *it == '\0') {
                    LOG_ERR("Middle parameter (" + p + ") not valid");
                    return NULL;
                }
            }
        } else {
            for (std::string::const_iterator it = p.begin(); it != p.end(); ++it) {
                if (*it == '\r' || *it == '\n' || *it == '\0') {
                    LOG_ERR("Trailing parameter (" + p + ") not valid");
                    return NULL;
                }
            }
        }
    }

    return new Command(msg.getCommand(), params);
}

// From the RFC:
// <command>  ::= <letter> { <letter> } | <number> <number> <number>
bool Command::isValidCommand(const std::string &cmd) {
    if ((cmd.length() == 3) && \
        std::isdigit(static_cast<unsigned char>(cmd[0])) &&
        std::isdigit(static_cast<unsigned char>(cmd[1])) &&
        std::isdigit(static_cast<unsigned char>(cmd[2])))
        return true;
    if (cmd.empty())
        return false;
    for (std::string::const_iterator it = cmd.begin(); it != cmd.end(); it++) {
        if (!std::isalpha(static_cast<unsigned char>(*it)))
            return false;
    }
    return true;
}


// <nick> = <letter> { <letter> | <number> | <special> }
// <letter> = 'a' ... 'z' | 'A' ... 'Z'
// <number> = '0' ... '9'
// <special> = '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
bool Command::isValidNick(const std::string &nickname) {
    if (nickname.empty())
        return false;
    if (nickname.size() > 9)
        return false;

    unsigned char first = (unsigned char)nickname[0];
    if (!std::isalpha(first))
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

const std::string& Command::getCmd(void) const {
    return this->command;
}

const std::vector<std::string>& Command::getParams(void) const {
    return this->param;
}
