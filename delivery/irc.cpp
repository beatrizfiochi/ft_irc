/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bfiochi- <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:50:15 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:50:15 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

bool isChannelName(const std::string &name) {
    if (name.empty())
        return false;
    if ((name[0] == '#') || (name[0] == '&'))
        return true;
    return false;
}
