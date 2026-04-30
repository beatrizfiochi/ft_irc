/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmoura-p <cmoura-p@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:45:17 by cmoura-p          #+#    #+#             */
/*   Updated: 2026/04/29 20:19:23 by cmoura-p         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client {
    private:
        int fd;

        std::string nickName;
        std::string userName;
        std::string realName;

        // Internal buffers for send and receive
        std::string read_buf;
        std::string write_buf;

        bool passOk;
        bool registered;

    public:
        Client();
        Client(int fd);
        Client(Client const &other);
        Client& operator=(Client const &rhs);
        ~Client();

        std::string& getReadBuf(void);
        std::string& getWriteBuf(void);
        int getFd(void) const;

// TODOs
//         void getUser();
//         void getNick();
//         void getReal();
//
//         void setNick();
//         void setUser();
//         void setReal();
//
// 	       void setRegister("true");
//         void setPassOK("true");
//
//         bool isRegistered();

};

#endif
