#include "lexer.hpp"
#include "ast.hpp"
#include <memory>
#include <stdexcept>
#include <vector>
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

    std::string type;
    bool has_builtin_base = false;

    //built in base is like a type with shi like unsigned int
    while (true) {
        switch (peek().type) {
        case token_type::CONST:
        case token_type::SIGNED:
        case token_type::UNSIGNED:
        case token_type::SHORT:
        case token_type::LONG:
            break;
        case token_type::INT:
        case token_type::CHAR:
        case token_type::FLOAT:
        case token_type::DOUBLE:
        case token_type::VOID:
            has_builtin_base = true;
            break;
        default:
            goto done_with_builtin_type;
        }

        if (!type.empty())
            type += ' ';

        type += advance().lexeme;
    }

done_with_builtin_type:
    if (!has_builtin_base && check(token_type::IDENTIFIER)) {
        if (!type.empty())
            type += ' ';
        type += advance().lexeme;

        while (match(token_type::SCOPE_RESOLUTION)) {
            type += "::";
            type += consume(token_type::IDENTIFIER, "expected type name after '::'").lexeme;
        }
    }

    if (type.empty())
        throw parse_error("expected type", peek().line, peek().column);

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

        if (check(token_type::LEFT_BRACE)) {
            function.body = parse_compound_statement();
            return std::make_unique<declaration>(std::move(function));
        }

        throw parse_error("expected ';' or function body after parameters", peek().line, peek().column);
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

std::vector<parameter> parser::parse_parameters() {

    std::vector<parameter> parameters;

    //no params
    if (check(token_type::RIGHT_PAREN)) {
        return parameters;
    }

    do {
        //get atleast first param then check cond
        std::string type = parse_type();

        const Token &name = consume(token_type::IDENTIFIER, "where param name :(");

        parameters.push_back({type, name.lexeme});

    } while(match(token_type::COMMA));

    return parameters;

}

stmt_ptr parser::parse_statement() {

    if (match(token_type::RETURN)) {
        return parse_return_statement();
    }

    if (check(token_type::LEFT_BRACE)) {
        return parse_compound_statement();
    }

    //temp easy soln
    expr_ptr expression = parse_expression();

    consume(token_type::SEMICOLON, "didnt find ; after expression");

    return std::make_unique<stmt>(expr_stmt{std::move(expression)}); //such an ugly yet crazy statement

}

stmt_ptr parser::parse_return_statement() {

    expr_ptr value = nullptr; //if void return type

   if (!check(token_type::SEMICOLON)) {
       value = parse_expression();
   }

   consume(token_type::SEMICOLON, "didnt find ; after return statemnet");

   return std::make_unique<stmt>(return_stmt{std::move(value)});

}

stmt_ptr parser::parse_compound_statement() {

    consume(token_type::LEFT_BRACE, "didnt find {");

    compound_stmt block;

    while(!check(token_type::RIGHT_BRACE) && !is_at_end()) {
        block.statements.push_back(parse_statement());
    }

    consume(token_type::RIGHT_BRACE, "didnt find } after func bod");

    return std::make_unique<stmt>(std::move(block));

}

expr_ptr parser::parse_equiality() {
    expr_ptr expression = parse_comparision();

    while (match(token_type::EQUAL_EQUAL) || match(token_type::BANG_EQUAL)) {

        std::string op = previous().lexeme;
        expr_ptr right = parse_comparision();

        expression = std::make_unique<expr>(binary_expr{
            op,
            std::move(expression),
            std::move(right)
        });

    }

    return expression;

}

expr_ptr parser::parse_factor() {

    expr_ptr expression = parse_unary();

    while (match(token_type::STAR) || match(token_type::SLASH) || match(token_type::PERCENT)) {

        std::string op = previous().lexeme;
        expr_ptr right = parse_unary();

        expression = std::make_unique<expr>(binary_expr{
            op,
            std::move(expression),
            std::move(right)
        });

    }

    return expression;

}

expr_ptr parser::parse_comparision() {

   expr_ptr expression = parse_terms();

   while (match(token_type::LESS) || match(token_type::LESS_EQUAL) || match(token_type::GREATER_EQUAL) || match(token_type::GREATER)) {

       std::string op = previous().lexeme;
       expr_ptr right = parse_terms();

       expression = std::make_unique<expr>(binary_expr {
           op,
           std::move(expression),
           std::move(right)
       });

   }

   return expression;


}

expr_ptr parser::parse_terms() {

    expr_ptr expression = parse_factor();

    while (match(token_type::PLUS) || match(token_type::MINUS)) {

        std::string op = previous().lexeme;
        expr_ptr right = parse_factor();

        expression = std::make_unique<expr>(binary_expr {

            op,
            std::move(expression),
            std::move(right)

        });

    }

    return expression;

}

expr_ptr parser::parse_unary() {

    if (match(token_type::BANG) || match(token_type::PLUS) || match(token_type::MINUS)) {

        std::string op = previous().lexeme;
        expr_ptr operand = parse_unary();

        return std::make_unique<expr>(unary_expr {

            op,
            std::move(operand)

        });


    }

    return parse_call();

}

expr_ptr parser::parse_call() {

    expr_ptr expression = parse_primary();

    while(match(token_type::LEFT_PAREN)) {

        std::vector<expr_ptr> args;

        if (!check(token_type::RIGHT_PAREN)) {
            do {
                args.push_back(parse_expression());
            }while (match(token_type::COMMA));
        }

        consume(token_type::RIGHT_PAREN, "didnt find ) after args");

        expression = std::make_unique<expr>(call_expr{

           std::move(expression),
           std::move(args)

        });


    }

    return expression;

}

expr_ptr parser::parse_primary() {

    if (match(token_type::NUMBER)) {
        return std::make_unique<expr>(num_expr{
            previous().lexeme
        });
    }

    if (match(token_type::STRING_LITERAL)) {
        return std::make_unique<expr>(str_expr{
           previous().lexeme
        });
    }

    if (match(token_type::IDENTIFIER)) {
        std::vector<std::string> parts;

        parts.push_back(previous().lexeme);

        while(match(token_type::SCOPE_RESOLUTION)) {
            const Token &name = consume(token_type::IDENTIFIER, "no identifier after ::");
            parts.push_back(name.lexeme);
        }

        return std::make_unique<expr>(name_expr{
            std::move(parts)
        });

    }

    if (match(token_type::LEFT_PAREN)) {
        expr_ptr expression = parse_expression();

        consume(token_type::RIGHT_PAREN, "expected )");
        return expression;
    }

    //failsafe
    throw parse_error("expected expression", peek().line, peek().column);

}

expr_ptr parser::parse_expression() {
    return parse_assignments();
}

expr_ptr parser::parse_assignments() {

    expr_ptr expression = parse_equiality();

    if (match(token_type::EQUAL)) {

        std::string op = previous().lexeme;
        expr_ptr value = parse_assignments();

        return std::make_unique<expr>(binary_expr{
            op,
            std::move(expression),
            std::move(value)
        });

    }

    return expression;


}


}
