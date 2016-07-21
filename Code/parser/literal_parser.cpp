#include "parser.h"

#include "../abstx/numbers.h"
#include "../abstx/str.h"
#include "../abstx/seq.h"
#include "../compile_time/compile_time.h"
#include <cstdlib>


std::shared_ptr<Literal> compile_int_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::INTEGER && !it->token.empty());
    ASSERT(!it.error);
    auto literal = std::shared_ptr<Literal>(new Literal());
    literal->start_token_index = it.current_index;
    literal->owner = parent_scope;
    literal->context = it->context;

    try {
        if (it->token[0] == '-') {
            int_least64_t v = std::stoll(it->token);
            literal->value.assign(std::shared_ptr<Type>(new Type_int()), v);
        } else {
            uint_least64_t v = std::stoull(it->token);
            literal->value.assign(std::shared_ptr<Type>(new Type_uint()), v);
        }
    } catch (std::out_of_range e) {
        log_error("Integer literal too large to fit!", it->context);
        literal->status = Parsing_status::FATAL_ERROR;
    }

    if (!is_error(literal->status))
        literal->status = Parsing_status::FULLY_RESOLVED;
    it.eat_token(); // eat the int token
    return literal;
}




std::shared_ptr<Literal> compile_float_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::FLOAT && !it->token.empty());
    ASSERT(!it.error);
    auto literal = std::shared_ptr<Literal>(new Literal());
    literal->start_token_index = it.current_index;
    literal->owner = parent_scope;
    literal->context = it->context;

    try {
        double v = std::stod(it->token);
        literal->value.assign(std::shared_ptr<Type>(new Type_float()), v);
    } catch (std::out_of_range e) {
        log_error("Float literal too large to fit!", it->context);
        literal->status = Parsing_status::FATAL_ERROR;
    }

    if (!is_error(literal->status))
        literal->status = Parsing_status::FULLY_RESOLVED;
    it.eat_token(); // eat the float token
    return literal;
}


std::shared_ptr<Literal> compile_bool_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->token == "true" || it->token == "false");

    auto literal = std::shared_ptr<Literal>(new Literal());
    literal->start_token_index = it.current_index;
    literal->owner = parent_scope;
    literal->context = it->context;

    bool v = (it->token == "true");
    literal->value.assign(std::shared_ptr<Type>(new Type_bool()), v);

    it.assert(Token_type::BOOL);

    if (!is_error(literal->status))
        literal->status = Parsing_status::FULLY_RESOLVED;

    return literal;
}




struct String_container {

    char* push(const std::string& s) {
        // FIXME: check if the string already exists in the list
        // if so, delete the pointer and return the old one

        int len = s.size();
        char* c = (char*)malloc(len+1); // '\0' terminated
        for (int i = 0; i < len; ++i) {
            c[i] = s[i];  // FIXME: better lexer support for utf8 than std::string
            if (s[i] == '\0') break;
        }
        c[len] = '\0';

        strings.push_back(c);
        return c;
    }

    ~String_container() {
        for (char* s : strings) {
            delete s;
        }
    }

private:
    std::vector<char*> strings;
};


String_container _string_container{}; // singleton

std::shared_ptr<Literal> compile_string_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::STRING && !it->token.empty());
    ASSERT(!it.error);
    auto literal = std::shared_ptr<Literal>(new Literal());
    literal->start_token_index = it.current_index;
    literal->owner = parent_scope;
    literal->context = it->context;

    int len = it->token.size();

    Str_header header;
    header.len = len;
    header.v = _string_container.push(it->token);

    literal->value.assign(std::shared_ptr<Type>(new Type_str()), header);

    literal->status = Parsing_status::FULLY_RESOLVED;
    it.eat_token(); // eat the string token
    return literal;
}








std::shared_ptr<Value_expression> compile_sequence_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::SYMBOL && it->token == "[");
    ASSERT(!it.error);

    auto seq = std::shared_ptr<Literal_seq>(new Literal_seq());

    seq->owner = parent_scope;
    seq->context = it->context;
    seq->start_token_index = it.current_index;

    seq->type = std::shared_ptr<Type_seq>(new Type_seq());
    seq->type->owner = seq;
    seq->type->context = seq->context;
    seq->type->start_token_index = seq->start_token_index;

    it.eat_token(); // eat the "[" token

    // literal syntax:
    // [int, size=3: 0, 0, 0]
    // [int: 0, 0, 0] // size inferred to 3
    // [size=3: 0, 0, 0] // type inferred to int (the type of the first element)
    // [0, 0, 0] // type inferred to int (the type of the first element), size inferred to 3

    // look ahead and search for ':' token

    bool has_size_data = false;
    int i = 0;

    int meta_index = it.find_matching_token(it.current_index, Token_type::SYMBOL, ":",
        "sequence literal", "", true, false); // search forward without logging errors
    bool has_metadata = it.error;

    if (has_metadata) {
        int first_member_index = meta_index + 1;

        // read meta data
        Token id, symbol;
        id = it.expect(Token_type::IDENTIFIER);
        if (!it.error) symbol = it.expect(Token_type::SYMBOL);

        if (!it.error && symbol.token == "," || symbol.token == ":") {
            // treat as type identifier
            auto type = parent_scope->get_type(id.token);
            if (type == nullptr) {
                log_error("Type not found: \""+id.token+"\" undeclared",id.context);
                seq->status = Parsing_status::SYNTAX_ERROR;

                //   Maybe: (this could log strange errors if multiple definitions of id)
                // auto t_id = parent_scope->get_identifier(id.token);
                // if (t_id != nullptr) {
                //     add_note("\""+id.token"\" is declared as a non-type variable here:", t_id->context);
                // }

            } else seq->type->type = type;
        }

        if (!it.error && symbol.token == ",") {
            // expect "size" token, then expect "=" token
            id = it.expect(Token_type::IDENTIFIER, "size");
            if (!it.error) symbol = it.expect(Token_type::SYMBOL, "=");
        }

        if (!it.error && id.token == "size" && symbol.token == "=") {
            // TODO: allow more complex expressions here, not just integer literal
            auto size_expr = compile_value_expression(it, parent_scope);
            ASSERT(size_expr != nullptr);
            if (size_expr->status == Parsing_status::FATAL_ERROR) {
                seq->status = Parsing_status::FATAL_ERROR;
                return seq;
            } else if (is_error(size_expr->status)) {
                seq->status = size_expr->status;
            } else {
                ASSERT(size_expr->status == Parsing_status::FULLY_RESOLVED);
                Value v = eval(size_expr);
                ASSERT(v.get_type() != nullptr);
                if (auto i_type = std::dynamic_pointer_cast<Type_int>(v.get_type())) { // if (v.type->is_integer_type())
                    seq->type->size = i_type->cpp_value(v.get_value());
                    has_size_data = true;
                } else {
                    log_error("Type mismatch: expected integer type but found "+v.get_type()->toS(), size_expr->context);
                    seq->status == Parsing_status::TYPE_ERROR;
                }
            }

            // const Token& size_literal = it.expect(Token_type::INTEGER);
            // if (!it.error) {
            //     // ASSERT(false, "FIXME: stoi")
            //     seq->type->size = std::stoi(size_literal.token);
            //     has_size_data = true;
            // } else {
            //     add_note("Only integer literals are supported for sequence literal size declaration");
            //     seq->status = Parsing_status::SYNTAX_ERROR;
            // }
        }

        if (!it.error) it.expect(Token_type::SYMBOL, ":");

        ASSERT(it.error || it.current_index == first_member_index);

        if (it.error) {
            add_note("In the meta data of sequence literal that started here: ", seq->context);
            seq->status = Parsing_status::SYNTAX_ERROR;
            it.current_index = first_member_index;
        }

        ASSERT(it.current_index == first_member_index);
    }

    // read value expressions until "]" token
    // check the type of each expression, log error if unable to get type
    // if seq->type is nullptr, get the type from the first value
    // else if v->type != seq->type log error

    bool first = true;
    std::vector<std::shared_ptr<Value_expression>> type_errors;
    while(true) {
        if (it->type == Token_type::SYMBOL && it->token == "]") {
            it.eat_token();
            break; // ok
        }
        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) {
            seq->status = Parsing_status::SYNTAX_ERROR;
            // try to recover
            it.current_index = it.find_matching_token(it.current_index, Token_type::SYMBOL, "]", "Seq_literal", "Missing \"]\"") + 1;
            if(it.error) {
                seq->status = Parsing_status::FATAL_ERROR;
                return seq; // not ok, undefined behaviour if we continue
            }
            break; // we didn't read the whole sequence, but that's ok. We log an error and continue with the next thing.
        }

        auto value_expr = compile_value_expression(it, parent_scope);
        ASSERT(value_expr != nullptr);
        value_expr->owner = seq;

        seq->members.push_back(value_expr);
        if (is_error(value_expr->status)) {
            seq->status = value_expr->status;
            if (is_fatal(value_expr->status)) {
                return seq; // not ok, undefined behaviour if we continue
            }
        } else {
            ASSERT(value_expr->status == Parsing_status::FULLY_RESOLVED); // FIXME: it might have dependencies?
            auto member_type = value_expr->get_type();
            ASSERT(member_type != nullptr);

            if (seq->type->type == nullptr) seq->type = member_type; // infer the type from the first member
            else if (seq->type->type != member_type) {
                type_errors.push_back(value_expr);
            }
        }

        first = false;
    }

    if (seq->type->type == nullptr) {
        log_error("Unable to infer the type of sequence literal", seq->context);
        seq->status = Parsing_status::TYPE_ERROR;
        seq->type->status = Parsing_status::TYPE_ERROR;
    } else if (!type_errors.empty()) {
        log_error("Incompatible types in sequence literal of type "+seq->type->type->toS(), seq->context);
        for (auto member : type_errors) {
            add_note("Member of type "+member->get_type()+" found here:",member->context);
        }
        seq->status = Parsing_status::TYPE_ERROR;
        seq->type->status = Parsing_status::TYPE_ERROR;
    } else {
        seq->type->status = seq->type->type->status;
    }

    if (!is_error(seq->status)) {
        seq->status = Parsing_status::FULLY_RESOLVED;
    }

    if (!has_size_data) {
        seq->type->size = seq->members.size();
    } else {
        if (seq->type->size < seq->members.size()) {
            // if size < members.size, give warning and cut off
            log_warning("Give size is less than the number of elements in the sequence literal. Exess members are ignored.", seq->context);
            seq->members.resize(seq->type->size);
        } else while (seq->type->size > seq->members.size()) {
            // if size > members.size, pad with default literals
            auto default_literal = std::shared_ptr<Literal>(new Literal());
            default_literal->owner = seq;
            default_literal->start_token_index = it.current_index - 1;
            default_literal->context = it.look_back(1).context;
            default_literal->status = Parsing_status::FULLY_RESOLVED;
            default_literal->value.alloc(seq->type->type);
            seq->members.push_back(default_literal);
        }
    }

    return seq;
}





