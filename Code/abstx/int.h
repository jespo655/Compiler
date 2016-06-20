#pragma once

#include "type.h"




struct Type_i8 : Type
{
    std::string toS() const override { return "i8"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_i16 : Type
{
    std::string toS() const override { return "i16"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_i32 : Type
{
    std::string toS() const override { return "i32"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_i64 : Type
{
    std::string toS() const override { return "i64"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

// struct Type_int : Type
// {
//     std::string toS() const override { return "int"; }
// };

#define Type_int Type_i64



struct Type_u8 : Type
{
    std::string toS() const override { return "u8"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_u16 : Type
{
    std::string toS() const override { return "u16"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_u32 : Type
{
    std::string toS() const override { return "u32"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

struct Type_u64 : Type
{
    std::string toS() const override { return "u64"; }
    std::shared_ptr<const Literal> get_default_value() const override;
};

// struct Type_uint : Type
// {
//     std::string toS() const override { return "uint"; }
// };

#define Type_uint Type_u64








#include "literal.h"

struct Literal_int : Literal
{
    int64_t value = 0;
    std::string toS() const override { return std::to_string(value); }
    std::shared_ptr<const Type> get_type() override;
};

struct Literal_uint : Literal
{
    uint64_t value = 0;
    std::string toS() const override { return std::to_string(value); }
    std::shared_ptr<const Type> get_type() override;
};




