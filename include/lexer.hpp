#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace cns{

//ai generated enum cuz i aint typing allat
  enum class token_type {

    END_OF_FILE,

    IDENTIFIER,
    NUMBER,
    STRING_LITERAL,
    CHAR_LITERAL,

    NAMESPACE,
    USING,
    INT,
    CHAR,
    FLOAT,
    DOUBLE,
    VOID,
    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,
    DO,
    BREAK,
    CONTINUE,
    SWITCH,
    CASE,
    DEFAULT,
    STRUCT,
    UNION,
    ENUM,
    TYPEDEF,
    CONST,
    STATIC,
    EXTERN,
    SIGNED,
    UNSIGNED,
    SHORT,
    LONG,
    SIZEOF,

    //operator
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,

    EQUAL,
    EQUAL_EQUAL,
    BANG,
    BANG_EQUAL,

    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,

    AMPERSAND,
    PIPE,
    CARET,
    TILDE,

    AMPERSAND_AMPERSAND,
    PIPE_PIPE,

    PLUS_PLUS,
    MINUS_MINUS,

    PLUS_EQUAL,
    MINUS_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    PERCENT_EQUAL,

    AMPERSAND_EQUAL,
    PIPE_EQUAL,
    CARET_EQUAL,

    LEFT_SHIFT,
    RIGHT_SHIFT,
    LEFT_SHIFT_EQUAL,
    RIGHT_SHIFT_EQUAL,

    ARROW,
    SCOPE_RESOLUTION,

    //bodmas
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,

    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    QUESTION,
    HASH,
    ELLIPSIS

  };

  struct Token {
    token_type type;
    std::string lexeme;

    int line;
    int column;
  };

  class LexError : public std::runtime_error {
    public:
      int line;
      int column;

    LexError(const std::string& message, int line, int column);
  };

  class Lexer {
    public:
      explicit Lexer(const std::string &source);

      std::vector<Token> tokenize();

    private:
      const std::string &source;

      std::vector<Token> tokens;

      size_t start = 0; //beginnign of current token
      size_t current = 0; //where we currently reading in that token

      int line = 1;
      int column = 1;

      int start_line = 1; //useful when we throw errs
      int start_column = 1;

      bool is_at_end() const;

      char peek() const;
      char peek_next() const;

      char advance();
      bool match(char expected);

      void scan_token();

      void scan_identifier();
      void scan_number();
      void scan_string(char quote);

      void skip_whitespace();
      void skip_line_comment();
      void skip_block_comment();

      void add_token(token_type type);

      static bool is_alpha(char c);
      static bool is_digit(char c);
      static bool is_alpha_num(char c);

      static token_type keyword_type(const std::string &text);
  };

}
