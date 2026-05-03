#include "irc.hpp"

bool isChannelName(const std::string &name) {
    if (name.empty())
        return false;
    if ((name[0] == '#') || (name[0] == '&'))
        return true;
    return false;
}
