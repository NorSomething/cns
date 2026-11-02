#pragma once

#include <memory>
#include <string>
#include <vector>
#include <variant>

namespace cns{

struct expr;
struct stmt;
struct declaration;

using expr_ptr = std::unique_ptr<expr>;
using stmt_ptr = std::unique_ptr<stmt>;
using decl_ptr = std::unique_ptr<declaration>;

//expressions
struct num_expr {
    std::string value;
};

struct str_expr {
    std::string value;
};

struct name_expr {
    std::vector<std::string> parts; //like a::b will become a,b
};

struct unary_expr {
    std::string op;
    expr_ptr operand;
};

struct binary_expr {
    std::string op;
    expr_ptr left;
    expr_ptr right;
};

struct call_expr {
    expr_ptr callee;
    std::pmr::vector<expr_ptr> args;
};

struct expr {
using Node = std::variant<
    num_expr,
    str_expr,
    name_expr,
    unary_expr,
    binary_expr,
    call_expr
    >;

    Node node;

    template<typename T>
    explicit expr(T value)
        : node(std::move(value)) {}
};

//src file here
struct translation_unit {
    std::vector<decl_ptr> declarations;
};

}
