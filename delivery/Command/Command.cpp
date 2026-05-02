#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include "Command.hpp"
#include "../log.hpp"

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

// From RFC:
// <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
Command *Command::parsing(const std::string &raw) {
    // "The prefix is used by servers to indicate the true origin of the message"
    std::string prefix;
    std::string command;
    size_t pos = 0;
    std::vector<std::string> param;

    // Check the end
    if ((raw.length() <= 2) || (std::string(raw.end() - 2, raw.end()) != "\r\n")){
        LOG_ERR("Message not valid. No crlf");
        return NULL;
    }

    if (raw[0] == ':') {
        // Process the prefix
        //TODO: Since this project does not have server-to-server communication. We probably don't need to implement
        //prefix. Check this.
        (void)prefix;
        return NULL;
    } else if (raw[0] == ' ') {
        LOG_ERR("Command (" + raw + ") not valid. Start with spaces");
        return NULL;
    }

    command = Command::getNextToken(raw, pos);
    if (!Command::isValidCommand(command)) {
        LOG_ERR("Command (" + command + ") not valid");
        return NULL;
    }
    do {
        std::string token = Command::getNextToken(raw, pos);
        if (token.empty())
            break;

        if (!Command::isValidParam(token)){
            LOG_ERR("Parameter (" + token + ") not valid");
            return NULL;
        }
        // Remove the :
        if (token[0] == ':') {
            token.erase(0, 1);
        }
        param.push_back(token);
    } while (true);

    return new Command(command, param);
}

// From the RFC:
// <SPACE>    ::= ' ' { ' ' }
size_t Command::skipSpaces(const std::string &str, size_t pos) {
    for (std::string::const_iterator i = str.begin() + pos; i != str.end(); i++){
        if (*i != ' ')
            return i - str.begin();
    }
    return str.end() - str.begin();
}

// From the RFC:
// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
//
// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                or NUL or CR or LF, the first of which may not be ':'>
// <trailing> ::= <Any, possibly *empty*, sequence of octets not including
//                  NUL or CR or LF>
std::string Command::getNextToken(const std::string &str, size_t &pos) {
    if (pos >= str.length()) {
        pos = str.length();
        return "";
    }

    size_t start = Command::skipSpaces(str, pos);
    if (start >= str.length()) {
        pos = str.length();
        return "";
    }

    if (str[start] == ':') {
        pos = str.length();
    } else {
        pos = str.find(' ', start);
        if (pos == std::string::npos)
            pos = str.length();
    }
    std::string result = std::string(str, start, pos - start);

    if ((result.length() > 2) && \
    (*(result.end() - 2) == '\r') && (*(result.end() - 1) == '\n')) {
        result.erase(result.end() - 2, result.end());
    }
    return result;
}

// From the RFC:
// <command>  ::= <letter> { <letter> } | <number> <number> <number>
bool Command::isValidCommand(const std::string &cmd) {
    if ((cmd.length() == 3) && \
        std::isdigit(static_cast<unsigned char>(cmd[0])) &&
        std::isdigit(static_cast<unsigned char>(cmd[1])) &&
        std::isdigit(static_cast<unsigned char>(cmd[2])))
        return true;
    for (std::string::const_iterator it = cmd.begin(); it != cmd.end(); it++) {
        if (!std::isalpha(static_cast<unsigned char>(*it)))
            return false;
    }
    return true;
}

// From the RFC:
// <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
//
// <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
//                or NUL or CR or LF, the first of which may not be ':'>
// <trailing> ::= <Any, possibly *empty*, sequence of octets not including
//                  NUL or CR or LF>
bool Command::isValidParam(const std::string &param) {
    // trailing
    if (param.length() == 0)
        return false;
    if (param[0] == ':') {
        if (param.length() >= 1) {
            for (std::string::const_iterator it = param.begin() + 1; it != param.end(); it++) {
                if ((*it == '\r') || (*it == '\n') || (*it == '\0'))
                    return false;
            }
        }
        return true;
    }

    // middle
    if (param.length() == 0)
        return false;
    for (std::string::const_iterator it = param.begin(); it != param.end(); it++) {
        if ((*it == ' ') || (*it == '\r') || (*it == '\n') || (*it == '\0'))
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

