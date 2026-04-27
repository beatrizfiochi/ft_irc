#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <string>
#include <vector>

class Command {
public:
    static Command *parsing(const std::string &raw);
private:
    std::string command;
    std::vector<std::string> param;

    Command(const std::string &command, const std::vector<std::string> &param);
    static size_t skipSpaces(const std::string &str, size_t pos);
    static std::string getNextToken(const std::string &str, size_t &pos);
    static bool isValidCommand(const std::string &cmd);
    static bool isValidParam(const std::string &param);
};

#endif // _COMMAND_HPP_
