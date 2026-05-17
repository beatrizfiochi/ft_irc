/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cmoura-p <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 12:33:23 by djunho            #+#    #+#             */
/*   Updated: 2026/05/17 12:49:52 by djunho           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstddef>
#include <string>
#include <vector>

class Client {
    private:
        std::size_t sessionId;
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
        Client(int fd, std::size_t sessionId);
        Client(int fd, const std::string &nickname, std::size_t sessionId = 0);
        Client(Client const &other);
        Client& operator=(Client const &rhs);
        ~Client();

        std::string& getReadBuf(void);
        std::string& getWriteBuf(void);
        std::size_t getSessionId(void) const;
        int getFd(void) const;

        std::string getNick(void) const;
        void setNick(const std::string &nick);

        void setRegister(bool flag);
        bool isRegistered(void) const;
        bool getPassFlag(void);
        void setPassFlag(bool flag);

        std::string getUser(void) const;
        void setUser(const std::string &username);

        std::string getReal(void) const;
        void setReal(const std::string &realname);
};

#endif
