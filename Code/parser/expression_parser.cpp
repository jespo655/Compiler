#include "expression_parser.h"
#include "../abstx/value_list.h"
#include "../abstx/scope.h"
#include "../abstx/seq.h"


std::shared_ptr<Value_expression> parse_value_list(Token_iterator& it, std::shared_ptr<Scope> parent_scope);
std::shared_ptr<Value_expression> parse_sequence_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope);




// parse_value_list: 43 lines
std::shared_ptr<Value_expression> parse_value_list(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    ASSERT(it->type == Token_type::SYMBOL && it->token == "(");
    ASSERT(!it.error);

    auto v_list = std::shared_ptr<Value_list>(new Value_list());
    v_list->owner = parent_scope;
    v_list->context = it->context;
    v_list->start_token_index = it.current_index;

    it.eat_token(); // eat the "(" token

    bool first = true;
    while(true) {
        if (it->type == Token_type::SYMBOL && it->token == ")") {
            it.eat_token();
            break; // ok
        }
        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) {
            v_list->status = Parsing_status::SYNTAX_ERROR;
            it.current_index = it.find_matching_token(it.current_index, Token_type::SYMBOL, ")", "Value_list", "Missing \")\"") + 1;
            ASSERT(!it.error); // we know that the ')' exists because the statement passed partial scope parsing
            break;
        }

        auto value_expr = parse_value_expression(it, parent_scope);
        ASSERT(value_expr != nullptr);
        value_expr->owner = v_list;
        v_list->expressions.push_back(value_expr);
        if (is_error(value_expr->status)) {
            v_list->status = value_expr->status;
        } else ASSERT(value_expr->status == Parsing_status::FULLY_RESOLVED);

        first = false;
    }

    if (!is_error(v_list->status)) {
        v_list->status = Parsing_status::FULLY_RESOLVED)
    }

    return v_list;
}




// parse_sequence_literal: 137 lines + some more (FIXME: type checking)
std::shared_ptr<Value_expression> parse_sequence_literal(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
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

    bool has_metadata = false;
    int first_member_index = it.current_index;
    bool has_size_data = false;
    int i = 0;

    while(true) {
        const Token& t = it.look_ahead(i++); // look ahead and increment i
        if (t.type == Token_type::SYMBOL) {
            if (t.token == ":") {
                has_metadata = true;
                first_member_index = it.current_index + i;
                break;
            } else if (t.token == "]") {
                has_metadata = false;
                break;
            }
        }
        ASSERT(!is_eof(t)); // since the statement passed partial scope parse, the statement must be well formed
    }
    if (has_metadata) {
        Token id, symbol;
        id = it.expect(Token_type::IDENTIFIER);
        if (!it.error) symbol = it.expect(Token_type::SYMBOL);

        if (!it.error && symbol.token == "," || symbol.token == ":") {
            // treat as type identifier
            auto type = parent_scope->get_type(id.token);
            if (type == nullptr) {
                log_error("Type not found: \""+id.token+"\" undeclared",id.context);

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
            const Token& size_literal = it.expect(Token_type::INTEGER);
            if (!it.error) {
                // ASSERT(false, "FIXME: stoi")
                seq->size = std::stoi(size_literal.token);
                has_size_data = true;
            } else {
                add_note("Only integer literals are supported for sequence literal size declaration");
            }
        }

        if (!it.error) it.expect(Token_type::SYMBOL, ":");

        ASSERT(it.error || it.current_index == first_member_index);

        if (it.error) {
            add_note("In the meta data of sequence literal that started here: ", seq->context);
            seq->status = Parsing_status::SYNTAX_ERROR;
        }

        it.current_index = first_member_index;
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
            it.current_index = it.find_matching_token(it.current_index, Token_type::SYMBOL, "]", "Seq_literal", "Missing \"]\"") + 1;
            ASSERT(!it.error); // we know that the ']' exists because the statement passed partial scope parsing
            break;
        }

        auto value_expr = parse_value_expression(it, parent_scope);
        ASSERT(value_expr != nullptr);
        value_expr->owner = seq;
        seq->members.push_back(value_expr);
        if (is_error(value_expr->status)) {
            seq->status = value_expr->status;
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
            add_note("Member of type "+member->type+" found here:",member->context);
        }
        seq->status = Parsing_status::TYPE_ERROR;
        seq->type->status = Parsing_status::TYPE_ERROR;
    } else {
        seq->type->status = seq->type->type->status;
    }

    if (!is_error(seq->status))
        seq->status = Parsing_status::FULLY_RESOLVED) {
    }

    return seq;
}











// reserved symbols:

// ()
// []
// {}
// :

// Kanske:
// .
// ->
// =











std::shared_ptr<Value_expression> parse_value_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope, int min_operator_prio)
{
    // Part 1:
    // if token is '(' read value list
    // else if token is '[' read seq literal
    // else if token is integer, float, bool or string literal, construct appropriate literal node
    // else if token is symbol or identifier, check if it's a prefix operator
    //      if it is, read value expression (with min_prio = op.prio), then construct prefix operator node
    // else if token is identifier, check if the identifier exists. If not, log error undeclared variable
    // if none of those applies, log error unexpected token

    // Part 2: suffix operators:
    // '(' function operator
    // '[' indexing operator
    // '.' getter operator
    // other identifier or symbol token: check if it's an infix operator with prio > min_prio
    //      if it is, read value expression (with min_prio = op.prio), then construct infix operator node
    return nullptr;
}


std::shared_ptr<Variable_expression> parse_variable_expression(Token_iterator& it, std::shared_ptr<Scope> parent_scope)
{
    auto context = it->context;
    auto value_expr = parse_value_expression(it, parent_scope);
    if (auto var_expr = std::dynamic_pointer_cast<Variable_expression>(value_expr)) {
        return var_expr; // ok
    }

    if (value_expr != nullptr)
        log_error("Found value expression in a context where a variable expression was expected",context);
    else
        log_error("Unable to parse variable expression",context);

    return nullptr;
}



































































/*

Parsing_status parse_comma_separated_values(
    std::shared_ptr<Scope> parent_scope,
    std::shared_ptr<Abstx_node> owner,
    std::vector<std::shared_ptr<Value_expression>>& expressions,
    Token_type  expected_closing_type,
    std::string expected_closing_token);
{
    Parsing_status status = Parsing_status::NOT_PARSED;
    bool first = true;
    while(true) {
        if (it->type == expected_closing_type && it->token == expected_closing_token) {
            it.eat_token();
            break; // ok
        }
        if (!first) it.expect(Token_type::SYMBOL, ",");
        if (it.error) {
            status = Parsing_status::SYNTAX_ERROR;
            it.current_index = find_matching_token(it.current_index, expected_closing_token, expected_closing_token, "Missing \""+expected_closing_token+"\"") + 1;
            ASSERT(!it.error); // we know that the token exists because the statement passed partial scope parsing
            break;
        }

        auto value_expr = parse_value_expression(it, parent_scope); // FIXME: add parent_scope to fn signature
        ASSERT(value_expr != nullptr);
        value_expr->owner = owner;
        expressions.push_back(value_expr);
        if (is_error(value_expr->status)) {
            status = value_expr->status;
        } else ASSERT(value_expr->status == Parsing_status::FULLY_RESOLVED);

        first = false;
    }
}
*/
