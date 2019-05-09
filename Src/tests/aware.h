#pragma once

#include "../utilities/debug.h"
#include "../utilities/unique_id.h"
#include "../utilities/assert.h"

// Class that's aware to copy/move constructions/assignments and deletions
struct Aware : public Serializable
{
    static int created; // incremented on any constructor
    static int alive; // incremented on any constructor; decremented on destructor or move from
    static int moved; // incremented on any move assignment or constructor
    static int copied; // incremented on any copy assignment or constructor

    int i = get_unique_id();
    int deleted = 0; // incremented on destructor. Can only be done once.
    // int copied_from = 0; // incremented on copy assignment from this. Illegal with const reference argument
    int moved_from = 0; // incremented on move assignment from this
    int copied_to = 0; // incremented on copy assignment to this
    int moved_to = 0; // incremented on move assignment to this

    Aware() { created++; alive++; }
    template<typename... Args>Aware(Args... args) { created++; alive++; }
    ~Aware() { ASSERT(!deleted); deleted++; if (!moved_from) alive--; }
    Aware(const Aware& o) { created++; alive++; *this = o; }
    Aware& operator=(const Aware& o) { ASSERT(!deleted); ASSERT(!o.deleted); ASSERT(!o.moved_from); copied++; copied_to++; return *this; }
    Aware(Aware&& o) { created++; alive++; *this = std::move(o); }
    Aware& operator=(Aware&& o) { ASSERT(!deleted); ASSERT(!o.deleted); ASSERT(!o.moved_from); moved++; moved_to++; o.moved_from++; alive--; return *this; }

    bool operator==(const Aware& o) const { return i == o.i; }
    bool operator!=(const Aware& o) const { return !(*this == o); }
    bool operator<(const Aware& o) const { return i < o.i; }
    bool operator>(const Aware& o) const { return o < *this; }
    bool operator>=(const Aware& o) const { return !(*this < o); }
    bool operator<=(const Aware& o) const { return !(o < *this); }

    std::string toS() const { return "Aware(" + std::to_string(i) + ")"; }

    static void reset() {
        Aware::created = 0;
        Aware::alive = 0;
        Aware::moved = 0;
        Aware::copied = 0;
    }
};
