#ifndef ASSERT_H
#define ASSERT_H

#include "debug.h" // defines DEBUG

#ifdef DEBUG
#include <stdexcept>
#include <sstream>
#include <iostream>
#endif

#ifdef DEBUG
#	define _GET_ASSERT_MACRO(_1,_2,ASSERT,...) ASSERT
#	define ASSERT(...) _GET_ASSERT_MACRO(__VA_ARGS__,_ASSERT2,_ASSERT1)(__VA_ARGS__)
#else
#	define ASSERT(...) void(0)
#endif

#ifdef DEBUG
#	define _ASSERT1(b) if (!(b))                                                        \
    do {                                                                                \
        std::ostringstream _assert_oss;                                                 \
        _assert_oss << __FILE__ << ":" << __LINE__                                      \
            << ": Assert failed: (" << #b << ")";                                       \
        std::cerr << _assert_oss.str() << std::endl;                                    \
        exit(1);                                                                        \
    } while(0)
#endif

#ifdef DEBUG
#	define _ASSERT2(b, msg) if (!(b))                                                   \
    do {                                                                                \
        std::ostringstream _assert_oss;                                                 \
        _assert_oss << __FILE__ << ":" << __LINE__                                      \
            << ": Assert failed: (" << #b << ")";                                       \
        _assert_oss << ": " << msg;                                                     \
        std::cerr << _assert_oss.str() << std::endl;                                    \
        exit(1);                                                                        \
    } while(0)
#endif

#endif

