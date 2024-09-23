#ifndef AST_H
#define AST_H

#include "treeprint.h"

// AST node tags
enum ASTKind {
  AST_ADD = 2000,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_INT_LITERAL,
  AST_UNIT,
  AST_STATEMENT,
  AST_VARDEF,
  AST_EQUAL, 
  AST_OR,
  AST_AND,
  AST_LESSER,
  AST_LESSER_EQUAL,
  AST_GREATER,
  AST_GREATER_EQUAL,
  AST_EQUAL_EQUAL,
  AST_NOT_EQUAL,
  // new node types
  AST_FUNC, 
  AST_IF,
  AST_ELSE,
  AST_WHILE,
  AST_LBRACE,
  AST_RBRACE,
  AST_COMMA,
  AST_PARAMS,
  AST_STMTS,
  AST_ARGS,
  AST_FUNC_CALL
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
