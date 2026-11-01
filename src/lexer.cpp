#include "lexer.hpp"

#include <cctype>
#include <unordered_map>

namespace cns {

LexError::LexError(
    const std::string &message,
    int line,
    int column
)
    : std::runtime_error(message), //baseclass
      line(line),
      column(column) {}

Lexer::Lexer(const std::string &source)
    : source(source) {}

bool Lexer::is_at_end() const {
    return current >= source.size();
}

//see cur char
char Lexer::peek() const {
    if (is_at_end())
        return '\0';

    return source[current];
}

char Lexer::peek_next() const {
    if (current + 1 >= source.size())
        return '\0';

    return source[current + 1];
}

//consume char
char Lexer::advance() {
    char c = source[current++];

    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }

    return c;
}

bool Lexer::match(char expected) {
    if (is_at_end())
        return false;

    if (peek() != expected)
        return false;

    advance();
    return true;
}

bool Lexer::is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::is_alpha_num(char c) {
    return is_alpha(c) || is_digit(c);
}

void Lexer::add_token(token_type type) {
    std::string text = source.substr(start, current-start);

    tokens.push_back({
        type,
        text,
        start_line,
        start_column
    });
}

//ai generated func cuz i aint typing allat
token_type Lexer::keyword_type(const std::string &text) {
    static const std::unordered_map<std::string, token_type> keywords = {
        {"namespace", token_type::NAMESPACE},
        {"using", token_type::USING},

        {"int", token_type::INT},
        {"char", token_type::CHAR},
        {"float", token_type::FLOAT},
        {"double", token_type::DOUBLE},
        {"void", token_type::VOID},

        {"return", token_type::RETURN},

        {"if", token_type::IF},
        {"else", token_type::ELSE},
        {"while", token_type::WHILE},
        {"for", token_type::FOR},
        {"do", token_type::DO},

        {"break", token_type::BREAK},
        {"continue", token_type::CONTINUE},

        {"switch", token_type::SWITCH},
        {"case", token_type::CASE},
        {"default", token_type::DEFAULT},

        {"struct", token_type::STRUCT},
        {"union", token_type::UNION},
        {"enum", token_type::ENUM},
        {"typedef", token_type::TYPEDEF},

        {"const", token_type::CONST},
        {"static", token_type::STATIC},
        {"extern", token_type::EXTERN},

        {"signed", token_type::SIGNED},
        {"unsigned", token_type::UNSIGNED},
        {"short", token_type::SHORT},
        {"long", token_type::LONG},

        {"sizeof", token_type::SIZEOF}
    };

    auto it = keywords.find(text);

    if (it != keywords.end())
        return it->second;

    return token_type::IDENTIFIER;
}

void Lexer::scan_identifier() {
    while (is_alpha_num(peek())) {
        advance();
    }

    std::string text = source.substr(
        start,
        current - start
    );

    add_token(keyword_type(text));
}

void Lexer::scan_number() {

    while(is_digit(peek())) {
        advance();
    }

    //deciman
    if (peek() == '.' && is_digit(peek_next())) {
        advance();

        while(is_digit(peek())) {
            advance();
        }

        //TODO: add support for scientific notation!!

        add_token(token_type::NUMBER);

    }

}

void Lexer::scan_string(char quote) {
    while (!is_at_end() && peek() != quote) {
        if (peek() == '\n') {
            throw LexError("non terminated string or char literal...", start_line, start_column);
        }

        if (peek() == '\\') {
            advance();

            if (is_at_end()) {
                break;
            }

            advance();

        }
        else {
            advance();
        }
    }

    if (is_at_end()) {
        throw LexError("non terminated string or char literal...", start_line, start_column);
    }

    advance();

    if (quote == '"')
        add_token(token_type::STRING_LITERAL);
    else
        add_token(token_type::CHAR_LITERAL);

}

void Lexer::skip_whitespace() {

    while(!is_at_end()) {
        char c = peek();

        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            advance();
        }
        else {
            break;
        }
    }
}

void Lexer::skip_line_comment() {
    while(!is_at_end() && peek() != '\n') {
       advance();
    }
}

void Lexer::skip_block_comment() {
    //advance(); //for starting * -> this is alr done by scan_token so no need here

    if (!is_at_end()) {
        if (peek() == '*' && peek_next() == '/') {
            advance();
            advance();
            return;
        }

        advance();
    }

    throw LexError("somethign went wrong when parsing block comment...", start_line, start_column);

}


void Lexer::scan_token() {
    char c = advance();

    switch (c) {
        case '(': add_token(token_type::LEFT_PAREN); break;
        case ')': add_token(token_type::RIGHT_PAREN); break;

        case '{': add_token(token_type::LEFT_BRACE); break;
        case '}': add_token(token_type::RIGHT_BRACE); break;

        case '[': add_token(token_type::LEFT_BRACKET); break;
        case ']': add_token(token_type::RIGHT_BRACKET); break;

        case ';': add_token(token_type::SEMICOLON); break;
        case ',': add_token(token_type::COMMA); break;
        case '?': add_token(token_type::QUESTION); break;
        case '#': add_token(token_type::HASH); break;

        case ':':
            if (match(':'))
                add_token(token_type::SCOPE_RESOLUTION);
            else
                add_token(token_type::COLON);
            break;

        case '.':
            if (match('.')) {
                if (match('.'))
                    add_token(token_type::ELLIPSIS);
                else
                    throw LexError("Expected third '.' in ellipsis", start_line, start_column);
            } else {
                add_token(token_type::DOT);
            }
            break;

        case '+':
            if (match('+'))
                add_token(token_type::PLUS_PLUS);
            else if (match('='))
                add_token(token_type::PLUS_EQUAL);
            else
                add_token(token_type::PLUS);
            break;

        case '-':
            if (match('-'))
                add_token(token_type::MINUS_MINUS);
            else if (match('='))
                add_token(token_type::MINUS_EQUAL);
            else if (match('>'))
                add_token(token_type::ARROW);
            else
                add_token(token_type::MINUS);
            break;

        case '*':
            if (match('='))
                add_token(token_type::STAR_EQUAL);
            else
                add_token(token_type::STAR);
            break;

        case '/':
            if (match('/')) {
                skip_line_comment();
            } else if (match('*')) {
                skip_block_comment();
            } else if (match('=')) {
                add_token(token_type::SLASH_EQUAL);
            } else {
                add_token(token_type::SLASH);
            }
            break;

        case '%':
            if (match('='))
                add_token(token_type::PERCENT_EQUAL);
            else
                add_token(token_type::PERCENT);
            break;

        case '=':
            if (match('='))
                add_token(token_type::EQUAL_EQUAL);
            else
                add_token(token_type::EQUAL);
            break;

        case '!':
            if (match('='))
                add_token(token_type::BANG_EQUAL);
            else
                add_token(token_type::BANG);
            break;

        case '<':
            if (match('<')) {
                if (match('='))
                    add_token(token_type::LEFT_SHIFT_EQUAL);
                else
                    add_token(token_type::LEFT_SHIFT);
            } else if (match('=')) {
                add_token(token_type::LESS_EQUAL);
            } else {
                add_token(token_type::LESS);
            }
            break;

        case '>':
            if (match('>')) {
                if (match('='))
                    add_token(token_type::RIGHT_SHIFT_EQUAL);
                else
                    add_token(token_type::RIGHT_SHIFT);
            } else if (match('=')) {
                add_token(token_type::GREATER_EQUAL);
            } else {
                add_token(token_type::GREATER);
            }
            break;

        case '&':
            if (match('&'))
                add_token(token_type::AMPERSAND_AMPERSAND);
            else if (match('='))
                add_token(token_type::AMPERSAND_EQUAL);
            else
                add_token(token_type::AMPERSAND);
            break;

        case '|':
            if (match('|'))
                add_token(token_type::PIPE_PIPE);
            else if (match('='))
                add_token(token_type::PIPE_EQUAL);
            else
                add_token(token_type::PIPE);
            break;

        case '^':
            if (match('='))
                add_token(token_type::CARET_EQUAL);
            else
                add_token(token_type::CARET);
            break;

        case '~':
            add_token(token_type::TILDE);
            break;

        case '"':
        case '\'':
            scan_string(c);
            break;

        default:
            if (is_alpha(c)) {
                scan_identifier();
            } else if (is_digit(c)) {
                scan_number();
            } else {
                throw LexError(std::string("Unexpected character: '") + c + "'", start_line, start_column);
            }
            break;
    }
}

//actual tokenizer!!!
std::vector<Token> Lexer::tokenize() {
    while (!is_at_end()) {
        skip_whitespace();

        if (is_at_end())
            break;

        start = current;
        start_line = line;
        start_column = column;

        scan_token();
    }

    tokens.push_back({
        token_type::END_OF_FILE,
        "",
        line,
        column
    });

    return tokens;
}
}
