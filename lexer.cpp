#include <map>
#include <cassert>
#include <cctype>
#include <string>
#include "cpputil.h"
#include "token.h"
#include "exceptions.h"
#include "lexer.h"

////////////////////////////////////////////////////////////////////////
// Lexer implementation
////////////////////////////////////////////////////////////////////////

Lexer::Lexer(FILE *in, const std::string &filename)
  : m_in(in)
  , m_filename(filename)
  , m_line(1)
  , m_col(1)
  , m_eof(false) {
}

Lexer::~Lexer() {
  // delete any cached lookahead tokens
  for (auto i = m_lookahead.begin(); i != m_lookahead.end(); ++i) {
    delete *i;
  }
  fclose(m_in);
}

Node *Lexer::next() {
  fill(1);
  if (m_lookahead.empty()) {
    SyntaxError::raise(get_current_loc(), "Unexpected end of input");
  }
  Node *tok = m_lookahead.front();
  m_lookahead.pop_front();
  return tok;
}

Node *Lexer::peek(int how_many) {
  // try to get as many lookahead tokens as required
  fill(how_many);

  // if there aren't enough lookahead tokens,
  // then the input ended before the token we want
  if (int(m_lookahead.size()) < how_many) {
    return nullptr;
  }

  // return the pointer to the Node representing the token
  return m_lookahead.at(how_many - 1);
}

Location Lexer::get_current_loc() const {
  return Location(m_filename, m_line, m_col);
}

// Read the next character of input, returning -1 (and setting m_eof to true)
// if the end of input has been reached.
int Lexer::read() {
  if (m_eof) {
    return -1;
  }
  int c = fgetc(m_in);
  if (c < 0) {
    m_eof = true;
  } else if (c == '\n') {
    m_col = 1;
    m_line++;
  } else {
    m_col++;
  }
  return c;
}

// "Unread" a character.  Useful for when reading a character indicates
// that the current token has ended and the next one has begun.
void Lexer::unread(int c) {
  ungetc(c, m_in);
  if (c == '\n') {
    m_line--;
    m_col = 99;
  } else {
    m_col--;
  }
}

void Lexer::fill(int how_many) {
  assert(how_many > 0);
  while (!m_eof && int(m_lookahead.size()) < how_many) {
    Node *tok = read_token();
    if (tok != nullptr) {
      m_lookahead.push_back(tok);
    }
  }
}

Node *Lexer::read_token() {
  int c, line = -1, col = -1;

  // skip whitespace characters until a non-whitespace character is read
  for (;;) {
    line = m_line;
    col = m_col;
    c = read();
    if (c < 0 || !isspace(c)) {
      break;
    }
  }

  if (c < 0) {
    // reached end of file
    return nullptr;
  }

  std::string lexeme;
  lexeme.push_back(char(c));

  if (isalpha(c)) {
    Node *tok = read_continued_token(TOK_IDENTIFIER, lexeme, line, col, isalnum);
    if (tok->get_str() == "var") {
      tok->set_tag(TOK_VAR);
    } else if (tok ->get_str() == "function") {
      tok->set_tag(TOK_FUNC);
    } else if (tok->get_str() == "if") {
      tok->set_tag(TOK_IF);
    } else if (tok->get_str() == "else") {
      tok->set_tag(TOK_ELSE);
    } else if (tok->get_str() == "while") {
      tok->set_tag(TOK_WHILE);
    }
    return tok;
  } else if (isdigit(c)) {
    return read_continued_token(TOK_INTEGER_LITERAL, lexeme, line, col, isdigit);
  } else {
    switch (c) {
    case '+':
      return token_create(TOK_PLUS, lexeme, line, col);
    case '-':
      return token_create(TOK_MINUS, lexeme, line, col);
    case '*':
      return token_create(TOK_TIMES, lexeme, line, col);
    case '/':
      return token_create(TOK_DIVIDE, lexeme, line, col);
    case '(':
      return token_create(TOK_LPAREN, lexeme, line, col);
    case ')':
      return token_create(TOK_RPAREN, lexeme, line, col);
    case ';':
      return token_create(TOK_SEMICOLON, lexeme, line, col);
    // for multi-char tokens, check one character ahead to determine token
    case '=': {
      if (check_next('=')) {
        return token_create(TOK_EQUAL_EQUAL, "==", line, col);
      }
      return token_create(TOK_EQUAL, "=", line, col);
    }
    case '<': {
      if (check_next('=')) {
        return token_create(TOK_LESSER_EQUAL, "<=", line, col);
      }
      return token_create(TOK_LESSER, "<", line, col);
    }
    case '>': {
      if (check_next('=')) {
        return token_create(TOK_GREATER_EQUAL, ">=", line, col);
      }
      return token_create(TOK_GREATER, ">", line, col);
    }
    case '|': {
      if (check_next('|')) {
        return token_create(TOK_OR, "||", line, col);
      }
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
    case '&': {
      if (check_next('&')) {
        return token_create(TOK_AND, "&&", line, col);
      }
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
    case '!': {
      if (check_next('=')) {
        return token_create(TOK_NOT_EQUAL, "!=", line, col);
      }
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
    case '{':
      return token_create(TOK_LBRACE, lexeme, line, col);
    case '}':
      return token_create(TOK_RBRACE, lexeme, line, col);
    case ',':
      return token_create(TOK_COMMA, lexeme, line, col);
    default:
      SyntaxError::raise(get_current_loc(), "Unrecognized character '%c'", c);
    }
  }
}

// Helper function to create a Node object to represent a token.
Node *Lexer::token_create(enum TokenKind kind, const std::string &lexeme, int line, int col) {
  Node *token = new Node(kind, lexeme);
  Location source_info(m_filename, line, col);
  token->set_loc(source_info);
  return token;
}

// Read the continuation of a (possibly) multi-character token, such as
// an identifier or integer literal.  pred is a pointer to a predicate
// function to determine which characters are valid continuations.
Node *Lexer::read_continued_token(enum TokenKind kind, const std::string &lexeme_start, int line, int col, int (*pred)(int)) {
  std::string lexeme(lexeme_start);
  for (;;) {
    int c = read();
    if (c >= 0 && pred(c)) {
      // token has finished
      lexeme.push_back(char(c));
    } else {
      if (c >= 0) {
        unread(c);
      }
      return token_create(kind, lexeme, line, col);
    }
  }
}

// determine whether next character matches 'target' character
bool Lexer::check_next(int target) {
  int ahead = read();
  if (ahead == target) {
    return true;
  }
  unread(ahead);
  return false;
}