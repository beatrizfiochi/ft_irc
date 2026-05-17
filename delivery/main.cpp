#include <iostream>
#include <sstream>
#include <string>
#include "log.hpp"
#include "Server.hpp"
#include "utils/Signal.hpp"
#include "utils/utils.hpp"

LOG_REGISTER(main);

static void usage(char *argv[]) {
    std::cout << "Usage: " << argv[0] << " <port> <password>\n";
    std::cout << "\t<port>: The listening port. Positive number only\n";
    std::cout << "\t<password>: The connection password\n";
}

static bool parseArg(int argc, char *argv[], unsigned int &port, std::string &password) {
    do {
        if (argc != 3)
            break;
        std::string port_s(argv[1]);
        if (!isNumber(port_s))
            break;
        std::istringstream iss(port_s);
        iss >> port; // num now holds the integer value 42
        password = std::string(argv[2]);
        return true;
    } while (0);
    usage(argv);
    return false;
}

int main(int argc, char *argv[]) {
    LOG_INF("IRC server started");

    setupSignalHandlers();

    unsigned int port;
    std::string passw;
    if (!parseArg(argc, argv, port, passw))
        return 1;

    LOG_DBG("port: " << port << "; password: " << passw);
    Server srv(port, passw);
    int ret = srv.run(&g_shutdownRequested);
    LOG_INF("IRC server stopped");
    return ret;
}
