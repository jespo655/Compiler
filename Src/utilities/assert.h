#ifndef ASSERT_H
#define ASSERT_H

#include "debug.h" // defines DEBUG

#ifdef DEBUG
#include <stdexcept>
#include <sstream>
#include <iostream>
#endif

#ifdef DEBUG
#	define GET_ASSERT_MACRO(_1,_2,ASSERT,...) ASSERT
#	define ASSERT(...) GET_ASSERT_MACRO(__VA_ARGS__,ASSERT2,ASSERT1)(__VA_ARGS__)
#else
#	define ASSERT(...)
#endif


#ifdef DEBUG
#	define ASSERT1(b) if (!(b))                                                         \
    {                                                                                   \
        std::ostringstream oss;                                                         \
        oss << __FILE__ << ":" << __LINE__ << ": Assert failed: (" << #b << ")";        \
        std::cerr << oss.str();                                                         \
        throw std::runtime_error(oss.str());                                            \
    }
#else
#	define ASSERT1(b)
#endif


#ifdef DEBUG
#	define ASSERT2(b, msg) if (!(b))                                                    \
    {                                                                                   \
        std::ostringstream _assert_oss;                                                         \
        _assert_oss << __FILE__ << ":" << __LINE__ << ": Assert failed: (" << #b << ")";        \
        _assert_oss << ": " << msg;                                                             \
        std::cerr << _assert_oss.str();                                                         \
        throw std::runtime_error(_assert_oss.str());                                            \
    }
#else
#	define ASSERT2(b, msg)
#endif


#endif
