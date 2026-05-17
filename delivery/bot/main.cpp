/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmoura-p <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:16 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:16 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <string>
#include "Bot.hpp"
#include "../log.hpp"
#include "../utils/Signal.hpp"
#include "../utils/utils.hpp"

LOG_REGISTER(bot_main);

static void usage(const char *prog_name) {
    std::cout << "Usage: " << prog_name
              << " <host> <port> <password> <nick> <channel>\n";
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        usage(argv[0]);
        return 1;
    }

    if (!isNumber(argv[2])) {
        usage(argv[0]);
        return 1;
    }

    LOG_INF("IRC bot started");

    unsigned int port;
    std::istringstream iss(argv[2]);
    iss >> port;

    setupSignalHandlers();

    Bot bot(argv[1], port, argv[3], argv[4], argv[5]);
    int ret = bot.run(&g_shutdownRequested);

    LOG_INF("IRC bot stopped");
    return ret;
}
