#include <iostream>
#include <sstream>
#include <string>
#include <csignal>
#include "log.hpp"
#include "Server.hpp"

LOG_REGISTER(main);

static volatile sig_atomic_t g_shutdown = 0;

static void signalHandler(int signum) {
    (void)signum;
    g_shutdown = 1;
}

static void usage(char *argv[]) {
    std::cout << "Usage: " << argv[0] << " <port> <password>\n";
    std::cout << "\t<port>: The listening port. Positive number only\n";
    std::cout << "\t<password>: The connection password\n";
}

static bool is_number(const std::string& s) {
    if (s.empty()) return false;

    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (!std::isdigit(*it)) {
            return false;
        }
    }
    return true;
}

static bool parseArg(int argc, char *argv[], unsigned int &port, std::string &password) {
    do {
        if (argc != 3)
            break;
        std::string port_s(argv[1]);
        if (!is_number(port_s))
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

    std::signal(SIGINT, signalHandler); // Ctrl+C
    std::signal(SIGTERM, signalHandler); // Kill
    std::signal(SIGQUIT, signalHandler); // Ctrl+backslash
    std::signal(SIGPIPE, SIG_IGN); // Client desconected during write

    unsigned int port;
    std::string passw;
    if (!parseArg(argc, argv, port, passw))
        return 1;

    LOG_DBG("port: " << port << "; password: " << passw);
    Server srv(port, passw);
    srv.run(&g_shutdown);

    LOG_INF("IRC server stopped");
    return 0;
}
