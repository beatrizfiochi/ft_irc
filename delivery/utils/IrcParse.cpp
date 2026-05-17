/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IrcParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bfiochi- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:16 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:16 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IrcParse.hpp"

// From the RFC:
// > these messages shall not exceed 512 characters in length, counting all
// > characters including the trailing CR-LF
#define IRC_MAX_LINE_SIZE 510

IrcMessage::IrcMessage(void)
    : prefix(""), command(""), params(), trailing(false) {}

IrcMessage::IrcMessage(const IrcMessage &other)
    : prefix(other.prefix), command(other.command),
      params(other.params), trailing(other.trailing) {}

IrcMessage &IrcMessage::operator=(const IrcMessage &other) {
    if (this != &other) {
        this->prefix = other.prefix;
        this->command = other.command;
        this->params = other.params;
        this->trailing = other.trailing;
    }
    return *this;
}

IrcMessage::~IrcMessage(void) {}

const std::string &IrcMessage::getPrefix(void) const {
    return this->prefix;
}

const std::string &IrcMessage::getCommand(void) const {
    return this->command;
}

const std::vector<std::string> &IrcMessage::getParams(void) const {
    return this->params;
}

bool IrcMessage::hasTrailing(void) const {
    return this->trailing;
}

void IrcMessage::setPrefix(const std::string &prefix) {
    this->prefix = prefix;
}

void IrcMessage::setCommand(const std::string &command) {
    this->command = command;
}

void IrcMessage::addParam(const std::string &param) {
    this->params.push_back(param);
}

void IrcMessage::setTrailing(bool trailing) {
    this->trailing = trailing;
}

// RFC: <SPACE> ::= ' ' { ' ' }
// Advance pos past any consecutive spaces.
static void skipSpaces(const std::string &line, std::string::size_type &pos) {
    while (pos < line.size() && line[pos] == ' ')
        ++pos;
}

bool tokenizeIrcLine(const std::string &line, IrcMessage &out) {
    out = IrcMessage();
    if (line.empty())
        return false;

    // RFC 1459 §2.3: a message starts with ':' (prefix) or directly with the
    // command. Leading whitespace is not valid.
    if (line[0] == ' ')
        return false;

    std::string::size_type pos = 0;

    // Optional prefix
    if (line[pos] == ':') {
        std::string::size_type sp = line.find(' ', pos + 1);
        if (sp == std::string::npos) {
            // Prefix but no command — invalid
            out.setPrefix(line.substr(pos + 1));
            return false;
        }
        out.setPrefix(line.substr(pos + 1, sp - pos - 1));
        pos = sp + 1;
    }

    skipSpaces(line, pos);
    if (pos >= line.size())
        return false;

    // Command
    std::string::size_type sp = line.find(' ', pos);
    if (sp == std::string::npos) {
        out.setCommand(line.substr(pos));
        return !out.getCommand().empty();
    }
    out.setCommand(line.substr(pos, sp - pos));
    if (out.getCommand().empty())
        return false;
    pos = sp;

    // Params
    while (pos < line.size()) {
        skipSpaces(line, pos);
        if (pos >= line.size())
            break;
        if (line[pos] == ':') {
            // Trailing — rest of line, may be empty
            out.addParam(line.substr(pos + 1));
            out.setTrailing(true);
            break;
        }
        std::string::size_type e = line.find(' ', pos);
        if (e == std::string::npos)
            e = line.size();
        out.addParam(line.substr(pos, e - pos));
        pos = e;
    }

    return true;
}

TokenizeResult tokenizeNextMessage(std::string &buf, IrcMessage &out) {
    out = IrcMessage();

    std::string::size_type end = buf.find("\r\n");
    if (end == std::string::npos) {
        // No frame yet. Guard against unbounded buffer growth: if we
        // already have more bytes queued than the RFC line cap allows,
        // the in-progress frame is doomed to fail the size check anyway.
        // Drop the bogus bytes now to avoid a memory-exhaustion DoS from
        // a peer that streams data without ever sending CRLF.
        if (buf.size() > IRC_MAX_LINE_SIZE) {
            buf.clear();
            return TOKENIZE_TOO_LONG;
        }
        return TOKENIZE_NO_FRAME;
    }

    std::string line = buf.substr(0, end);
    buf.erase(0, end + 2);

    if (line.size() > IRC_MAX_LINE_SIZE)
        return TOKENIZE_TOO_LONG;
    if (line.empty())
        return TOKENIZE_EMPTY;
    if (!tokenizeIrcLine(line, out))
        return TOKENIZE_MALFORMED;
    return TOKENIZE_OK;
}

std::string nickFromPrefix(const std::string &prefix) {
    std::string::size_type bang = prefix.find('!');
    if (bang == std::string::npos)
        return prefix;
    return prefix.substr(0, bang);
}
