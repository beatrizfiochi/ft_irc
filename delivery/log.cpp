#include <string>
#include "log.hpp"
#ifdef ENABLE_TIMESTAMP
    #include <ctime>
#endif
#ifdef ENABLE_COUNT
    #include <sstream>
#endif

#ifdef ENABLE_COUNT
    static unsigned long counter = 0;
#endif

std::string log_header(std::string module, std::string lvl) {
    std::string hdr;

#ifdef ENABLE_TIMESTAMP
    time_t now = time(0);
    struct tm *ltm = localtime(&now);

    char buffer[80];
    // %Y: Year, %m: Month, %d: Day, %H: Hour, %M: Minute, %S: Second
    // This format produces: "2026-04-24 10:42:05"
    if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm)) {
        hdr += buffer;
    }
#endif

#ifdef ENABLE_COUNT
    std::stringstream ss;
    ss << counter++;
    hdr += " " + ss.str() + " ";
#endif

    hdr += "[" + lvl + "] " + module + ": ";
    return hdr;
}

