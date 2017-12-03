#pragma once

#include "type.h"
#include "pointers.h"
#include "dynamic_seq.h"

#include <string>

/*
An operator can be used as both infix and prefix operator.
All specializations of an operator must be known at compile time.
No generics are allowed. (for now)
*/

struct Operator_metadata {
    CB_String symbol;
    CB_Int prefix_prio;
    CB_Int infix_prio;
};

struct CB_Operator
{
    static CB_Type type;
    CB_Owning_pointer<Operator_metadata> metadata;
    CB_Dynamic_seq<CB_Function> infix_fns;
    CB_Dynamic_seq<CB_Function> prefix_fns;

    std::string toS() const {
        return "operator("+metadata->symbol.toS()+", p="+metadata->prefix_prio.toS()+", i="+metadata->infix_prio.toS()+")";
    }

    CB_Operator() { metadata = alloc(Operator_metadata()); }
    CB_Operator(const CB_String& symbol, const CB_Int& prefix_prio=0, const CB_Int& infix_prio=0) {
        metadata = alloc(Operator_metadata());
        metadata->symbol = symbol;
        metadata->prefix_prio = prefix_prio;
        metadata->infix_prio = infix_prio;
    }
    ~CB_Operator() {}

    CB_Operator& operator=(const CB_Operator& fn) {
        metadata = alloc<Operator_metadata>(*fn.metadata);
        infix_fns = fn.infix_fns;
        prefix_fns = fn.prefix_fns;
        return *this;
    }
    CB_Operator(const CB_Operator& fn) { *this = fn; }

    CB_Operator& operator=(CB_Operator&& fn) {
        infix_fns = std::move(fn.infix_fns);
        prefix_fns = std::move(fn.prefix_fns);
        metadata = std::move(fn.metadata);
        return *this;
    }
    CB_Operator(CB_Operator&& fn) { *this = fn; }


    void add_prefix(const CB_Function& fn) {
        // verify types
        ASSERT(fn.metadata->in_args.size == 1);
        ASSERT(fn.metadata->out_args.size <= 1);
        CB_Type arg_t = fn.metadata->in_args[0].type;
        for (auto& pfn : prefix_fns) {
            ASSERT(pfn.metadata->in_args[0].type != arg_t, "Multiple definition of prefix operator "+metadata->symbol.toS()+" "+arg_t.toS());
        }
        prefix_fns.add(fn);
    }

    void add_infix(const CB_Function& fn) {
        // verify types
        ASSERT(fn.metadata->in_args.size == 2);
        ASSERT(fn.metadata->out_args.size <= 1);
        CB_Type lhs_t = fn.metadata->in_args[0].type;
        CB_Type rhs_t = fn.metadata->in_args[1].type;
        for (auto& pfn : infix_fns) {
            ASSERT(pfn.metadata->in_args[0].type != lhs_t || pfn.metadata->in_args[1].type != rhs_t,
                "Multiple definition of infix operator "+lhs_t.toS()+" "+metadata->symbol.toS()+" "+rhs_t.toS());
        }
        infix_fns.add(fn);
    }

    CB_Function get_prefix(const CB_Type& arg_t) {
        for (auto& pfn : prefix_fns) {
            if (pfn.metadata->in_args[0].type == arg_t)
                return pfn;
        }
        ASSERT(false, "No prefix operator"+metadata->symbol.toS()+" "+arg_t.toS());
        return CB_Function();
    }

    CB_Function get_infix(const CB_Type& lhs_t, const CB_Type& rhs_t) {
        for (auto& ifn : infix_fns) {
            if(ifn.metadata->in_args[0].type == lhs_t && ifn.metadata->in_args[1].type == rhs_t)
                return ifn;
        }
        ASSERT(false, "No infix operator "+lhs_t.toS()+" "+metadata->symbol.toS()+" "+rhs_t.toS());
        return CB_Function();
    }

};
CB_Type CB_Operator::type = CB_Type("operator");


