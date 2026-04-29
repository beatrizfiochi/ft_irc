#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <map>
#include <string>
#include <vector>

class Server {
public:
    Server(void);
    Server(unsigned int port, std::string passw);
    ~Server(void);
    int run(void);
private:
    unsigned int port;
    std::string passw;
    int srv_socket;
    int epollfd;
    //TODO: After implementing Clients, move it to there
    std::map<int, std::string> clientBuffers;
    std::map<int, std::string> clientOutBuffers;

    int openServerSocket(void);
    int listenEvents(void);
    int addNewClient(void);
    int receiveData(int fd);
    int flushReplyBuffer(int fd);
    int setWriteInterest(int fd, bool enabled);
    int sendReply(int fd, int err,
                  const std::vector<std::string> &params,
                  const std::string &trailing);
    void disconnectClient(int fd);
    void processBufferedMessages(int fd);
};
#endif // _SERVER_HPP_
