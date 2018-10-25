//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_PORTMACROS_HPP
#define SHAMC_PORTMACROS_HPP

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#endif //SHAMC_PORTMACROS_HPP
