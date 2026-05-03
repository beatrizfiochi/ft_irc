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
        Client(int fd, std::string nickname);
        Client(Client const &other);
        Client& operator=(Client const &rhs);
        ~Client();

        std::string& getReadBuf(void);
        std::string& getWriteBuf(void);
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
