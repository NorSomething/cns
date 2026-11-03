#include "lexer.hpp"
#include "ast.hpp"
#include <stdexcept>
#include "parser.hpp"

namespace cns {

//constructor
parser::parser(const std::vector<Token> &tokens) : tokens(tokens) {}

parse_error::parse_error(const std::string &message, int line, int column)
    : std::runtime_error(message),
    line(line),
    column(column) {}

bool parser::is_at_end() const {
    return peek().type == token_type::END_OF_FILE;
}

const Token &parser::peek() const {
    return tokens[current];
}

const Token &parser::previous() const{
    return tokens[current - 1];
}

const Token &parser::advance() {
    if (!is_at_end())
        current++;

    return previous();
}

bool parser::check(token_type type) const {
    if (is_at_end())
        return type == token_type::END_OF_FILE;

    return peek().type == type;
}

bool parser::match(token_type type) {
    if (!check(type))
        return false;

    advance();
    return true;
}

const Token &parser::consume(token_type type, const std::string &message) {

    if (check(type)) //here match() is being called to check token
        return advance();

    throw parse_error(message, peek().line, peek().column);


}

translation_unit parser::parse() {
    translation_unit unit;

    while(!is_at_end()) {
        unit.declarations.push_back(parse_declaration()); //parse tokens group  by group
    }

    return unit;

}

decl_ptr parser::parse_declaration() {

    if (match(token_type::NAMESPACE)) {
        return parse_namespace();
    }
    else if (is_type_start()) {
        return parse_func_or_variable();
    }
    else {
        throw parse_error("no declaration found", peek().line, peek().column);
    }

}

//ai gen func cuz again, i aint typing allat
// helper func
bool parser::is_type_start() const {
    switch (peek().type) {
        case token_type::INT: case token_type::CHAR: case token_type::FLOAT:
        case token_type::DOUBLE: case token_type::VOID: case token_type::IDENTIFIER:
        case token_type::CONST: case token_type::SIGNED: case token_type::UNSIGNED:
        case token_type::SHORT: case token_type::LONG:
            return true;
        default:
            return false;
    }
}

std::string parser::parse_type() {
    if (!is_type_start())
        throw parse_error("expected type", peek().line, peek().column);


    //note; wtf is this func.. feels like something is wrong here but i cant put my finger on it
    //i think its eating identifiers as types aswell

    std::string type;
    while (is_type_start()) {
        if (!type.empty())
            type += ' ';

        type += advance().lexeme;

        if (previous().type == token_type::IDENTIFIER)
            break;
    }
    return type;
}

decl_ptr parser::parse_func_or_variable() {
    const std::string type = parse_type();
    const Token &name = consume(token_type::IDENTIFIER, "expected declaration name");
    const std::string name_text = name.lexeme;

    if (match(token_type::LEFT_PAREN)) {
        function_declr function;
        function.return_type = type;
        function.name = name_text;

        if (!check(token_type::RIGHT_PAREN)) {

            //getting all parameters
            do {
                parameter param;
                param.type = parse_type();
                param.name = consume(token_type::IDENTIFIER, "expected parameter name").lexeme;
                function.parameters.push_back(std::move(param));
            } while (match(token_type::COMMA));

        }

        consume(token_type::RIGHT_PAREN, "expected ')' after parameters");

        if (match(token_type::SEMICOLON)) {
            return std::make_unique<declaration>(std::move(function));
        }
        throw parse_error("expected ';' after function prototype", peek().line, peek().column);
    }

    variable_declr variable;
    variable.type = type;
    variable.name = name_text;
    consume(token_type::SEMICOLON, "expected ';' after variable declaration");

    return std::make_unique<declaration>(std::move(variable));
}

decl_ptr parser::parse_namespace() {
    const Token &name = consume(token_type::IDENTIFIER, "expected namespace name");
    consume(token_type::LEFT_BRACE, "explected { after namespace name");

    namespace_declr ns;
    ns.name = name.lexeme;

    while (!check(token_type::RIGHT_BRACE) && !is_at_end()) {
        ns.declarations.push_back(parse_declaration());
    }

    consume(token_type::RIGHT_BRACE, "Expected } after namespace name");

    return std::make_unique<declaration>(std::move(ns));

}

}
