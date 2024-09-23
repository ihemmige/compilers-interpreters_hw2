#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint() {
}

ASTTreePrint::~ASTTreePrint() {
}

std::string ASTTreePrint::node_tag_to_string(int tag) const {
  switch (tag) {
    case AST_ADD:
      return "ADD";
    case AST_SUB:
      return "SUB";
    case AST_MULTIPLY:
      return "MULTIPLY";
    case AST_DIVIDE:
      return "DIVIDE";
    case AST_VARREF:
      return "VARREF";
    case AST_INT_LITERAL:
      return "INT_LITERAL";
    case AST_UNIT:
      return "UNIT";
    case AST_STATEMENT:
      return "STATEMENT";
    case AST_VARDEF:
      return "VARDEF";
    case AST_EQUAL:
      return "ASSIGN";
    case AST_OR:
      return "LOGICAL_OR";
    case AST_AND:
      return "LOGICAL_AND";
    case AST_LESSER:
      return "LESS_THAN";
    case AST_LESSER_EQUAL:
      return "LESS_THAN_EQUAL";
    case AST_GREATER:
      return "GREATER_THAN";
    case AST_GREATER_EQUAL:
      return "GREATER_THAN_EQUAL";
    case AST_EQUAL_EQUAL:
      return "EQUAL";
    case AST_NOT_EQUAL:
      return "NOT EQUAL";
    case AST_FUNC:
      return "FUNCTION";
    case AST_IF:
      return "IF";
    case AST_ELSE:
      return "ELSE";
    case AST_WHILE:
      return "WHILE";
    case AST_LBRACE:
      return "{";
    case AST_RBRACE:
      return "}";
    case AST_COMMA:
      return ",";
    case AST_PARAMS:
      return "PARAMETER_LIST";
    case AST_STMTS:
      return "STATEMENT_LIST";
    case AST_ARGS:
      return "ARGUMENT_LIST";
    case AST_FUNC_CALL:
      return "FUNC_CALL";
    default:
      RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
