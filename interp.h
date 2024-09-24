#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"
#include <unordered_set>
class Node;
class Location;

class Interpreter {
private:
  Node *m_ast;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:
  void check_vars(std::unordered_set<std::string>& var_set, Node* parent);
  Value execute_node(Environment& env, Node* node);
  static Value intrinsic_readint(Value args[], unsigned arg_ct, const Location &loc, Interpreter *interp);
  static Value intrinsic_print(Value args[], unsigned arg_ct, const Location &loc, Interpreter *interp);
  static Value intrinsic_println(Value args[], unsigned arg_ct, const Location &loc, Interpreter *interp);
};

#endif // INTERP_H
