/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmoura-p <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:33:22 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:49:52 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _BOT_HPP_
#define _BOT_HPP_

#include <string>
#include <csignal>
#include "../utils/IrcParse.hpp"

class Bot {
public:
    Bot(const std::string &host, unsigned int port,
        const std::string &password, const std::string &nick,
        const std::string &channel);
    Bot(const Bot &other);
    Bot &operator=(const Bot &other);
    ~Bot(void);

    int run(volatile sig_atomic_t *shutdown_flag);

private:
    std::string host;
    unsigned int port;
    std::string password;
    std::string nick;
    std::string channel;
    int fd;
    std::string read_buf;
    std::string write_buf;
    volatile sig_atomic_t *shutdown_flag;

    int  connectToServer(void);
    int  registerWithServer(void);
    int  sendLine(const std::string &line);
    int  flushWrite(void);
    int  receiveData(void);
    void processMessages(void);
    void handleMessage(const IrcMessage &msg);
    void handlePing(const std::string &trailing);
    void handlePrivmsg(const std::string &sender_nick,
                       const std::string &target,
                       const std::string &message);
    void replyTo(const std::string &reply_target, const std::string &msg);
};

#endif // _BOT_HPP_
