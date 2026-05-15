#include "Signal.hpp"

volatile sig_atomic_t g_shutdownRequested = 0;

static void shutdownHandler(int signum) {
    (void)signum;
    g_shutdownRequested = 1;
}

void setupSignalHandlers(void) {
    std::signal(SIGINT, shutdownHandler);   // Ctrl+C
    std::signal(SIGTERM, shutdownHandler);  // kill
    std::signal(SIGQUIT, shutdownHandler);  // Ctrl+backslash
    std::signal(SIGPIPE, SIG_IGN);          // peer closed during write
}
