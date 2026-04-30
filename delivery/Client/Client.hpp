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

        std::string getNick(void) const;
        void setNick(const std::string &nick);

// TODOs
//         void getUser();
//         void getReal();
//
//         void setUser();
//         void setReal();
//
// 	       void setRegister("true");
//         void setPassOK("true");
//
//         bool isRegistered();

};

#endif
