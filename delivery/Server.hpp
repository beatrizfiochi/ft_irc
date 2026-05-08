#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <map>
#include <set>
#include <string>
#include <csignal>
#include "Client/Client.hpp"
#include "Channel/Channel.hpp"

class Server {
public:
    Server(void);
    Server(unsigned int port, std::string passw);
    ~Server(void);
    int run(volatile sig_atomic_t *shutdown_flag); // *shutdown_flag is the pointer of signals

    int sendGenericMsg(Client &source, Client &target,
                       const std::string &target_msg, const std::string &msg);
    int sendPrivMsg(Client &source,
                Client &target,
                const std::string &msg);
    int sendReply(int fd, int err,
                  const std::string &cmd,
                  const std::string &trailing);
    bool checkPass(const std::string &passw) const;
    std::string getHostname(void) const;
    void addClientToNickList(Client &c, const std::string &oldname);
    Client *getClient(const std::string &nick);

    // Channel integration
    Channel* getChannel(const std::string &name);
    Channel& createChannel(const std::string &name);
    void removeChannel(const std::string &name);
    void kickClient(int fd, Channel &ch);

    int broadcastMsg(Channel& ch, Client &sender, const std::string &cmd,
                     const std::string& msg, int excludeFd = -1);
    int broadcastTo(const std::set<int> &fds, Client &sender,
                    const std::string &cmd, const std::string &msg,
                    int excludeFd = -1, bool echoToSender = false);
    std::set<int> getCommonChannelFds(int fd) const;

private:
    unsigned int port;
    std::string passw;
    int srv_socket;
    int epollfd;
    volatile sig_atomic_t *shutdown_flag; 
    // Client map <fd, Client>
    std::map<int, Client>           client;
    std::map<std::string, Client*>  nickList;
    // Channel integration
    std::map<std::string, Channel> channels;

    int openServerSocket(void);
    int listenEvents(void);
    int addNewClient(void);
    int receiveData(int fd);
    int flushReplyBuffer(int fd);
    int setWriteInterest(int fd, bool enabled);
    void connectClient(int fd);
    void disconnectClient(int fd);
    int processBufferedMessages(int fd);

    void removeClientFromChannels(int fd);

    void shutdown(void);
};
#endif // _SERVER_HPP_
