//
// Created by evan on 8/27/18.
//

#ifndef CACHED_ALIGNER_LOGGER_H
#define CACHED_ALIGNER_LOGGER_H

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

// terminal output colors
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

#include <iostream>
#include <chrono>

enum LOG_LEVEL {
    SILENT = 0,
    INFO = 1,
    WARN = 2,
    DEBUG = 3
};

// Logging flags
// TODO: solve variable reset due to not being truly global (singleton?)
static LOG_LEVEL log_level (LOG_LEVEL::DEBUG);
static bool show_time = true;

// Logging flag setters SHOULD ONLY BE CALLED FROM MAIN
inline void set_log_level(LOG_LEVEL level){log_level = level;}
inline void set_show_time(bool show){show_time = show;}

/*
 *  Log info level events.
 */
inline void log_info(const std::string& what) {
    if (log_level > LOG_LEVEL::SILENT){
        std::cout << what << std::endl;
    }
}

/*
 *  Log warn level events.
 */
inline void log_warn(const std::string& what) {
    if (log_level > LOG_LEVEL::INFO){
        std::cout << COLOR_YELLOW << what << COLOR_RESET << std::endl;
    }
}

/*
 *  Log debug level events.
 */
inline void log_debug(const std::string& what) {
    if (log_level > LOG_LEVEL::WARN){
        std::cout << what << std::endl;
    }
}

/*
 *  Wrap a void function or lambda in a timer function
 *
 *  WARNING: only works with void functions! Will evaluate any function but not return anything!
 */
template <typename Lambda, typename... Args>
inline void timer(const std::string& what, Lambda&& func, Args... args) {
    // how it works:
    // func is a function or lambda function that takes some number of arguments and returns void
    // args is a list of zero or more arguments you wish to pass to func
    // this function will record the time before and after evaluating func with the optional parameters
    // what simply describes what is being timed
    auto start = std::chrono::steady_clock::now();
    func(args...);  // &args... expands to a list of zero or more args
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    if (log_level > LOG_LEVEL::SILENT && show_time) {
        std::cout << what << " in: " << duration.count() << "s" << std::endl;
    }
}

#endif //CACHED_ALIGNER_LOGGER_H
