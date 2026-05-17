/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djunho <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:33:22 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:49:52 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

bool isNumber(const std::string &s);
std::vector<std::string> split(const std::string& str, char delimiter);

#endif // UTILS_HPP
