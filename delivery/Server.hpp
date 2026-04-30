#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <map>
#include <string>
#include <vector>
#include "Client/Client.hpp"

class Server {
public:
    Server(void);
    Server(unsigned int port, std::string passw);
    ~Server(void);
    int run(void);

    int sendReply(int fd, int err,
                  const std::string &cmd,
                  const std::string &trailing);
    bool checkPass(const std::string &passw) const;
private:
    unsigned int port;
    std::string passw;
    int srv_socket;
    int epollfd;
    // Client map <fd, Client>
    std::map<int, Client>      client;

    int openServerSocket(void);
    int listenEvents(void);
    int addNewClient(void);
    int receiveData(int fd);
    int flushReplyBuffer(int fd);
    int setWriteInterest(int fd, bool enabled);
    void disconnectClient(int fd);
    void processBufferedMessages(int fd);
};
#endif // _SERVER_HPP_
