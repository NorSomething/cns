#pragma once

#include "lexer.hpp"
#include "ast.hpp"

#include <vector>
#include <stdexcept>

namespace cns {

class parse_error : public std::runtime_error {
    public:
        int line;
        int column;

        parse_error(const std::string &message, int line, int column);

};

class parser {
    public:
        explicit parser(const std::vector<Token> &tokens);
        translation_unit parse();

    private:
        const std::vector<Token> &tokens;
        size_t current = 0;

        //token navigation
        bool is_at_end() const;
        const Token &peek() const;
        const Token &previous() const;

        const Token &advance();

        bool check(token_type type) const;
        bool match(token_type type);

        const Token &consume(token_type type, const std::string &message);

        //some declarations
        decl_ptr parse_declaration();
        decl_ptr parse_namespace();
        decl_ptr parse_func_or_variable();

        //funcs
        std::vector<parameter> parse_parameters();

        //statements
        stmt_ptr parse_statement();
        stmt_ptr parse_compound_statement();
        stmt_ptr parse_return_statement();
        stmt_ptr parse_variable_statement();

        expr_ptr parse_expression();
        expr_ptr parse_assignments();
        expr_ptr parse_equiality();
        expr_ptr parse_comparision();
        expr_ptr parse_terms();
        expr_ptr parse_factor();
        expr_ptr parse_unary();
        expr_ptr parse_call();
        expr_ptr parse_primary();

        //some helpers (type handling)
        std::string parse_type();
        bool is_type_start() const;
};

}
