#ifndef ASSERT_H
#define ASSERT_H

#include "debug.h" // defines DEBUG

#ifdef DEBUG
#include <stdexcept>
#include <sstream>
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
        oss << "Assert failed in file " << __FILE__ << ", line " << __LINE__;           \
        throw std::runtime_error(oss.str());                                            \
    }
#else
#	define ASSERT1(b)
#endif


#ifdef DEBUG
#	define ASSERT2(b, msg) if (!(b))                                                    \
    {                                                                                   \
        std::ostringstream oss;                                                         \
        oss << "Assert failed in file " << __FILE__ << ", line " << __LINE__;           \
        oss << ": " << msg;                                                             \
        throw std::runtime_error(oss.str());                                            \
    }
#else
#	define ASSERT2(b, msg)
#endif


#endif