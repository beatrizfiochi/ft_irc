#include <iostream>
#include <sstream>
#include <string>

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
    std::cout << "ft_irc server started\n";

    unsigned int port;
    std::string passw;
    if (!parseArg(argc, argv, port, passw))
        return 1;

    std::cout << "DEBUG: port: " << port << "; password: " << passw << "\n";
    return 0;
}
