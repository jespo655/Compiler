#pragma once

#include "type.h"
#include "literal.h"

/*
A pointer points to a value.
The value that a pointer points to is always owned by a scope.
At the end of scope, the value is deleted.

alloc($T t) allocates space on the heap for the object t, and memcopies t to that place.
alloc returns a pointer owned by the local scope.

T* marks a pointer that doesn't own its object. At the end of scope, nothing happens.
T*! marks a pointer that does own its object. At the end of scope, the object is deleted.

If a T*! is assigned another T*!, the ownership switches. The original pointer then turns into a T*.
A T*! can be returned or passed into a function that specifies T*! type. The ownership
of the pointer is then returned or passed to the function.

If a T*! is assigned a T*, no ownership is changed. The new T*! will turn into a T*, because
there was no ownership to take over to begin with.
*/


/*
There are no pointer literals - pointers can only be created through alloc() and by
taking the address of other objects.

p := alloc(0); // p becomes an int*!
p := &t; // p points to t and becomes a T* (it's impossible to take over the ownership of a variable on the stack)


foo := fn(i:int)->int*! { return alloc(i); }    // foo returns a valid int*!
bar := fn(p:int*) { }                           // after bar(p) is called, p will be unaffected
baz := fn(p:int*!) { }                          // after baz(p) is called, p is deleted (the function takes ownership of p)
*/

struct Type_pointer : Type
{
    std::shared_ptr<const Type> type;
    bool owned = false; // if true, then the value is deleted at the end of scope

    std::string toS() const override {
        ASSERT(type != nullptr);
        return type->toS() + "*" + (owned? "!" : "") ;
    }

    std::shared_ptr<Literal> get_default_value() const override
    {
        ASSERT(false, "Type_pointer::get_default_value() should never be called - pointer literals doesn't exist");
        return nullptr;
    }

    int byte_size() override { return sizeof(void*); }
};


/*
Maybe: pointer auto casts down to it's held value type

foo := fn(int) {}
a := alloc(int(0));

foo(*a); // valid
foo(a); // valid?
*/