#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <map>
#include <string>
#include "Client/Client.hpp"

class Server {
public:
    Server(void);
    Server(unsigned int port, std::string passw);
    ~Server(void);
    int run(void);

    int sendMsg(Client &source,
                Client &target,
                const std::string &msg);
    int sendReply(int fd, int err,
                  const std::string &cmd,
                  const std::string &trailing);
    bool checkPass(const std::string &passw) const;
    std::string getHostname(void) const;
    void addClientToNickList(Client &c, const std::string &oldname);
    Client *getClient(const std::string &nick);
private:
    unsigned int port;
    std::string passw;
    int srv_socket;
    int epollfd;
    // Client map <fd, Client>
    std::map<int, Client>           client;
    std::map<std::string, Client*>  nickList;

    int openServerSocket(void);
    int listenEvents(void);
    int addNewClient(void);
    int receiveData(int fd);
    int flushReplyBuffer(int fd);
    int setWriteInterest(int fd, bool enabled);
    void connectClient(int fd);
    void disconnectClient(int fd);
    void processBufferedMessages(int fd);
};
#endif // _SERVER_HPP_
