#ifndef ASSERT_H
#define ASSERT_H

#include "debug.h" // defines DEBUG

#ifdef DEBUG
#include <stdexcept>
#include <sstream>
#include <iostream>
#endif

#ifdef DEBUG
#define _GET_ASSERT_MACRO(_1,_2,ASSERT,...) ASSERT
#define ASSERT(...) _GET_ASSERT_MACRO(__VA_ARGS__,_ASSERT2,_ASSERT1)(__VA_ARGS__)

#define _ASSERT1(b) do { if (!(b)) {                                                \
    std::ostringstream _assert_oss;                                                 \
    _assert_oss << std::endl << __FILE__ << ":" << __LINE__                         \
        << ": Assert failed: (" << #b << ")";                                       \
    std::cerr << _assert_oss.str() << std::endl;                                    \
    throw -1;                                                                       \
}} while(0)

#define _ASSERT2(b, msg) do { if (!(b)) {                                           \
    std::ostringstream _assert_oss;                                                 \
    _assert_oss << std::endl << __FILE__ << ":" << __LINE__                         \
        << ": Assert failed: (" << #b << ")";                                       \
    _assert_oss << ": " << msg;                                                     \
    std::cerr << _assert_oss.str() << std::endl;                                    \
    throw -1;                                                                       \
}} while(0)

#define QUIET_ASSERT(b) do { if (!(b)) throw -1; } while(0)

#else
#define ASSERT(...) void(0)
#define QUIET_ASSERT(...) void(0)
#endif

#define ENSURE_CLEAN_EXIT(statement) do { try { statement; } catch (...) { exit(1); } } while(0)

#endif

