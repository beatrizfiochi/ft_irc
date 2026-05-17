/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Signal.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bfiochi- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:33:22 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:49:52 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SIGNAL_HPP_
#define _SIGNAL_HPP_

#include <csignal>

// Set to 1 by SIGINT/SIGTERM/SIGQUIT handlers. Long-running event loops
// should poll this and shut down cleanly when it becomes non-zero.
extern volatile sig_atomic_t g_shutdownRequested;

// Install handlers for SIGINT/SIGTERM/SIGQUIT (set g_shutdownRequested) and
// ignore SIGPIPE (so a write to a peer that closed its end returns EPIPE
// instead of terminating the process).
void setupSignalHandlers(void);

#endif // _SIGNAL_HPP_
