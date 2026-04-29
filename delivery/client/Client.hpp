/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmoura-p <cmoura-p@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:45:17 by cmoura-p          #+#    #+#             */
/*   Updated: 2026/04/29 14:32:09 by cmoura-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include "../Server.hpp"

// CLIENT ERRORS
#define ERR_NEEDMOREPARAMS 461
#define ERR_ALREADYREGISTERED 462
#define ERR_NICKCOLLISION 436
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NONICKNAMEGIVEN 431
#define ERR_NOTDEFINED 999

class Client {
    private:

        int fd;

        std::string nickName;
        std::string userName;
        std::string realName;

        bool passOk;
        bool registered;

    public:
        Client();
        Client(Client const &other);
        Client& operator=(Client const &rhs);
        ~Client();

    /*  void getUserN();
        void getNickN();
        void getRealN();
    */

        int handlePass(const std::vector<std::string> &args, const std::string &serverPassword);
        int handleNick(const std::vector<std::string> &args);
        int handleUser(const std::vector<std::string> &args);
//		int handleQuit(const std::vector<std::string> &args);

        static bool isValidNick(const std::string &nickname);
};

#endif
