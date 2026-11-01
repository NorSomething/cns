//ai generated file i just wanted it to print tokens to test stuff

#include "lexer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

const char* tokenName(cns::token_type type) {
    using T = cns::token_type;

    switch (type) {
        case T::END_OF_FILE: return "EOF";
        case T::IDENTIFIER: return "IDENTIFIER";
        case T::NUMBER: return "NUMBER";
        case T::STRING_LITERAL: return "STRING";
        case T::CHAR_LITERAL: return "CHAR";

        case T::NAMESPACE: return "NAMESPACE";
        case T::USING: return "USING";

        case T::INT: return "INT";
        case T::CHAR: return "CHAR_TYPE";
        case T::FLOAT: return "FLOAT";
        case T::DOUBLE: return "DOUBLE";
        case T::VOID: return "VOID";
        case T::RETURN: return "RETURN";

        case T::SCOPE_RESOLUTION: return "SCOPE_RESOLUTION";

        case T::LEFT_BRACE: return "LEFT_BRACE";
        case T::RIGHT_BRACE: return "RIGHT_BRACE";
        case T::LEFT_PAREN: return "LEFT_PAREN";
        case T::RIGHT_PAREN: return "RIGHT_PAREN";

        case T::PLUS: return "PLUS";
        case T::MINUS: return "MINUS";
        case T::STAR: return "STAR";
        case T::SLASH: return "SLASH";

        case T::EQUAL: return "EQUAL";
        case T::EQUAL_EQUAL: return "EQUAL_EQUAL";

        case T::SEMICOLON: return "SEMICOLON";
        case T::COMMA: return "COMMA";

        default: return "OTHER";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: cns <source.cns>\n";
        return 1;
    }

    std::ifstream file(argv[1]);

    if (!file) {
        std::cerr << "Could not open file: " << argv[1] << '\n';
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string source = buffer.str();

    try {
        cns::Lexer lexer(source);

        auto tokens = lexer.tokenize();

        for (const auto &token : tokens) {
            std::cout
                << token.line << ":"
                << token.column << " "
                << tokenName(token.type) << " "
                << "'" << token.lexeme << "'\n";
        }
    } catch (const cns::LexError &error) {
        std::cerr
            << argv[1] << ":"
            << error.line << ":"
            << error.column << ": error: "
            << error.what() << '\n';

        return 1;
    }

    return 0;
}
