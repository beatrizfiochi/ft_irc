/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djunho <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:16 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:16 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Signal.hpp"

volatile sig_atomic_t g_shutdownRequested = 0;

static void shutdownHandler(int signum) {
    (void)signum;
    g_shutdownRequested = 1;
}

void setupSignalHandlers(void) {
    std::signal(SIGINT, shutdownHandler);   // Ctrl+C
    std::signal(SIGTERM, shutdownHandler);  // kill
    std::signal(SIGQUIT, shutdownHandler);  // Ctrl+backslash
    std::signal(SIGPIPE, SIG_IGN);          // peer closed during write
}
