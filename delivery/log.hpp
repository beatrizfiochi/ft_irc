#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>
#include <string>

// Enable the LOG module
#if !defined(ENABLE_LOG)
    #define ENABLE_LOG  0
#endif

// Check the log level
#ifndef ENABLE_LOG_LVL
    #define ENABLE_LOG_LVL 3
#endif

// Enable or disable the timestamp in the logs
#ifndef ENABLE_TIMESTAMP
    #define ENABLE_TIMESTAMP    1
#endif

// Enable or disable the log counter
#ifndef ENABLE_COUNT
    #define ENABLE_COUNT    1
#endif

#ifndef ENABLE_COLOR
    #define ENABLE_COLOR    1
#endif

#if ENABLE_COLOR == 1
    // ANSI Escape Codes
    const std::string CLR_RESET = "\033[0m";
    const std::string CLR_ERR   = "\033[31m";    // RED
    const std::string CLR_WRN   = "\033[33m";    // YELLOW
    const std::string CLR_INF   = "\033[32m";    // GREEN
    const std::string CLR_DBG   = "\033[36m";    // CYAN
#else
    #define CLR_RESET
    #define CLR_ERR
    #define CLR_WRN
    #define CLR_INF
    #define CLR_DBG
#endif

#ifdef ENABLE_LOG
    #define LOG_REGISTER(module)       static const std::string log_module(#module)

    std::string log_header(std::string module, std::string lvl);
    #if ENABLE_LOG_LVL >= 1
        #define LOG_ERR(msg) std::cout << CLR_ERR << log_header("ERR", log_module) << msg << CLR_RESET << std::endl
    #else
        #define LOG_ERR(msg)
    #endif
    #if ENABLE_LOG_LVL >= 2
        #define LOG_WRN(msg) std::cout << CLR_WRN << log_header("WRN", log_module) << msg << CLR_RESET << std::endl
    #else
        #define LOG_WRN(msg)
    #endif
    #if ENABLE_LOG_LVL >= 3
        #define LOG_INF(msg) std::cout << CLR_INF << log_header("INF", log_module) << msg << CLR_RESET << std::endl
    #else
        #define LOG_INF(msg)
    #endif
    #if ENABLE_LOG_LVL >= 4
        #define LOG_DBG(msg) std::cout << CLR_DBG << log_header("DBG", log_module) << msg << CLR_RESET << std::endl
    #else
        #define LOG_DBG(msg)
    #endif
#else // ENABLE_LOG
    #define LOG_REGISTER(module)

    #define LOG_ERR(msg)
    #define LOG_WRN(msg)
    #define LOG_INF(msg)
    #define LOG_DBG(msg)
#endif // ENABLE_LOG

#endif // _LOG_HPP_
