






bool is_assignment_operator(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    // TODO: make this function a search in a sorted vector of tokens

    if (t.token == ":") return true;
    if (t.token == "=") return true;
    if (t.token == "+=") return true;
    if (t.token == "-=") return true;
    if (t.token == "*=") return true;
    if (t.token == "/=") return true;
    if (t.token == "%=") return true;

    return false;
}


bool is_closing_symbol(const Token& t)
{
    if (t.type != Token_type::SYMBOL) return false;

    if (t.token == ")") return true;
    if (t.token == "]") return true;
    if (t.token == "}") return true;

    return false;
}











// type_token: ":"-token
// assignment_token: "="-token, or other assignment operators if
// end_token: ";"-token
// returns true if error
bool examine_statement(Token const* it, Token const*& type_token, Token const*& assignment_token, Token const*& end_token)
{
    ASSERT(it != nullptr);

    bool found_assignment = false;
    bool found_type = false;
    bool errors = false;

    while(true) {
        if (it->type == Token_type::UNKNOWN || is_closing_symbol(*it)) {
            log_error("Missing \";\" after statement",it->context);
            add_note("Expected \";\" before \""+it->token+"\"");
            return errors;
        }
        if (it->type == Token_type::SYMBOL) {
            if (it->token == ";") {
                end_token = it;
                return errors;
            }
            else if (it->token == "(") it = read_paren(tokens,it);
            else if (it->token == "[") it = read_bracket(tokens,it);
            else if (it->token == "{") it = read_brace(tokens,it);
            else if (it->token == ":") {
                if (found_type) {
                    log_error("Multiple \":\" operators in statement",it->context);
                    add_note("First found here: ",assignment_token->context);
                    add_note("Expected \";\" after each assignment.");
                    errors = true;
                }
                if (found_assignment) {
                    log_error("Unexpected \":\" operator found after assignment.");
                    add_note("Assignment operator found here: ",assignment_token->context);
                    add_note("\":\" must always come before \"=\".");
                    errors = true;
                }
                found_type = true;
                type_token = it;
            }
            else if (is_assignment_operator(*it)) {
                if (found_assignment) {
                    log_error("Multiple assignment operators in statement",it->context);
                    add_note("First found here: ",assignment_op_token->context);
                    add_note("Expected \";\" after each assignment.");
                    errors = true;
                }
                if (found_type && it->token != "=") {
                    log_error("Unexpected assignment token: \""+it->token+"\"",it->context);
                    add_note("Only \"=\" are allowed after \":\".")
                    add_note("\":\" token found here: ",type_token->context);
                    errors = true;
                }
                found_assignment = true;
                assignment_token = it;
            }
            if (it == nullptr) return true; // can't continue after error from read_paren and Co.
        }
        it++;
    }
}




// stops when there are no more ";"-tokens in the current scope
// returns true if error
bool read_static_scope_statements(Token const* it, Static_scope& scope)
{
    ASSERT(it != nullptr);

    bool errors = false;

    while(it->type != Token_type::UNKNOWN && !is_closing_symbol(*it)) {

        Token const* type_token, assignment_token, end_token = nullptr;
        if (examine_statement(it, type_token, assignment_token, end_token);) {
            // We encountered an error, but if possible, continue going for the rest of the statements.
            // Getting several errors at the same time makes debugging easier.
            if (end_token == nullptr) return true; // not possible to go further
            it = end_token+1;
            errors = true;
            continue;
        }

        /**
            Possibilities:

            1) only ":" token, no assignment
            2) both ":" and "=" tokens
            3) only assignment token
            4) no ":" nor assignment tokens
        */
        else if (type_token != nullptr) {
            ASSERT(type_token->type == Token_type::SYMBOL);
            ASSERT(type_token->token == ":");
            // Read definition

            unique_ptr<Definition> definition{new Definition()};

            // TODO
            // read lhs
            cerr << "should read lhs of definition, but not yet implemented" << endl;

            Token const* type_end = end_token;
            if (assignment_token != nullptr) type_end = assignment_token;

            if (type_end > type_token+1) {

                // TODO
                // Read the type identifiers. Add them to the scope if not already added.
                // Update the corresponding identifiers with type info.
                cerr << "should read type identifier list, but not yet implemented" << endl;

            } else if (assignment_token == nullptr) {
                log_error("Unable to infer the types of identifiers; missing type identifiers after \":\"",end_token->context);
                errors = true;
            }

            if (assignment_token != nullptr) {
                ASSERT(assignment_token->type == Token_type::SYMBOL);
                ASSERT(assignment_token->token == "=");

                // TODO
                // Also read rhs. Enforce static_scope.
                cerr << "should read rhs of definition, but not yet implemented" << endl;
            }

            // TODO: add all defined identifiers to the scope
            // if they are already defined -> error
            scope->statements.push_back(definition);

        } else if (assignment_token != nullptr) {
            ASSERT(assignment_token->type == Token_type::SYMBOL);
            ASSERT(is_assignment_operator(*assignment_token));

            // Assignment

            log_error("Variable assignment not allowed in a static scope!",assignment_token->context);
            errors = true;
        } else {

            // Can be a function call or an anonymous scope
            unique_ptr<Evaluated_value> value{nullptr};
            if (read_evaluated_value(it,value)) {
                errors = true;
            } else {
                ASSERT(value != nullptr);
                if (Scope* s = dynamic_cast<Scope*>(value.get())) {
                    cerr << "read anonymous scope, but don't know how to handle it yet" << endl;
                } else if {Function_call* fc = dynamic_cast<Function_call*>(value.get())} {
                    log_error("Function call not allowed in static scope!",it->context);
                    add_note("Use \"#run\" to create a dynamic scope (NYI)");
                    errors = true;
                }
            }
        }
        it = end_token+1;
    }
    return errors;
}












std::unique_ptr<Scope> parse_tokens(const std::vector<Token>& tokens)
{
    if (tokens.empty()) return nullptr;
    unique_ptr<Scope> global_scope{new Scope()};
    global_scope->statements = read_statement_list(tokens,&tokens[0],global_scope.get(),false);
    if (global_scope->statements.empty()) {
        // since t is not empty we expect at least one statement. -> errors -> return nulllptr.
        return nullptr;
    }
    Token const * t = global_scope->statements.back()->end_token+2; // +1 is ";" token. +2 should be eof
    if (t < &tokens.back()) {
        log_error("Extra token after statements in global scope: Found extra token \""+t->token+"\"",t->context);
        return nullptr;
    }
    global_scope->start_token = &tokens[0];
    global_scope->end_token = &tokens.back();
    return global_scope;
}

std::unique_ptr<Scope> parse_file(const std::string& file)
{
    return parse_tokens(get_tokens_from_file(file));
}

std::unique_ptr<Scope> parse_string(const std::string& string)
{
    return parse_tokens(get_tokens_from_string(string));
}
