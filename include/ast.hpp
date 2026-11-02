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

struct return_stmt {
    expr_ptr value;
};

struct expr_stmt {
    expr_ptr expression;
};

struct var_declr_stmt {
    std::string type;
    std::string name;
    expr_ptr initializer;
};

struct compound_stmt {
    std::vector<stmt_ptr> statements;
};

struct stmt {
    using Node = std::variant<
        return_stmt,
        expr_stmt,
        var_declr_stmt,
        compound_stmt
    >;

    Node node;

    template<typename T>
    explicit stmt(T value)
        : node(std::move(value)) {}
};

//declrs
struct parameter {
    std::string type;
    std::string name;
};

struct function_declr {
    std::string return_type;
    std::string name;

    std::vector<parameter> parameters;

    stmt_ptr body; //null here then is just func prototype
};

struct variable_declr {
    std::string type;
    std::string name;
    expr_ptr initializer;
};

struct namespace_declr {
    std::string name;
    std::vector<decl_ptr> declarations;
};

struct declaration {
    using Node = std::variant<
    namespace_declr,
    function_declr,
    variable_declr
    >;

    Node node;
    template<typename T>
    explicit declaration(T value)
        : node(std::move(value)) {}
};


//src file here
struct translation_unit {
    std::vector<decl_ptr> declarations;
};

}
