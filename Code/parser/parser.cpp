#include "parser.h"
#include "lexer.h"
#include "parsing_status.h"



std::shared_ptr<Global_scope> parse_file(const std::string& file, const std::string& name) // default name is the file name
{
    return parse_tokens(get_tokens_from_file(file), name);
}

std::shared_ptr<Global_scope> parse_string(const std::string& string, const std::string& name) // FIXME: add string context
{
    return parse_tokens(get_tokens_from_string(string), name);
}

std::shared_ptr<Global_scope> parse_tokens(const std::vector<Token>& tokens, const std::string& name)
{
    std::shared_ptr<Global_scope> global_scope{new Global_scope(tokens)};
    global_scope->parse_partially();
    return global_scope;
}




Parsing_status Global_scope::parse_partially()
{
    return Parsing_status::NOT_PARSED;
}
