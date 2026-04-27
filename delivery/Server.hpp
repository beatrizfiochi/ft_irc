#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <string>

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

    int openServerSocket(void);
    int listenEvents(void);
    int addNewClient(void);
    int receiveData(int fd);
};
#endif // _SERVER_HPP_
