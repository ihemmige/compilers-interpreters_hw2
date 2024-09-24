#include <cassert>
#include <algorithm>
#include <memory>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"
#include <unordered_set>
#include <iostream>

Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
  delete m_ast;
}

// recursively ensures any varrefs are preceded by a vardef
void Interpreter::check_vars(std::unordered_set<std::string>& var_set, Node* parent) {
  if (parent->get_tag() == AST_VARDEF) { // new variable definition
    var_set.insert(parent->get_kid(0)->get_str());
  } else if (parent->get_tag() == AST_VARREF && var_set.find(parent->get_str()) == var_set.end()){ // undefined variable
      SemanticError::raise(parent->get_loc(), "Undefined variable %s", parent->get_str().c_str());
  }
  for (auto it = parent->cbegin(); it != parent->cend(); ++it) {
    Node* child = *it;
    if (child->get_tag() == AST_STMTS) {
      std::unordered_set<std::string> scoped_var_set(var_set);
      check_vars(scoped_var_set, child);
    } else {
      check_vars(var_set, child);
    }
  } 
}

void Interpreter::analyze() {
  std::unordered_set<std::string> var_set = {"print", "println", "readint"}; // defined variables set
  check_vars(var_set, m_ast);
}

Value Interpreter::execute() {
  std::unique_ptr<Environment> env(new Environment());
  // bind intrinsic functions
  env->bind_func("print", Value(&intrinsic_print));
  env->bind_func("println", Value(&intrinsic_println));
  env->bind_func("readint", Value(&intrinsic_readint));

  Value result;
  // execute each statement node in the tree
  for (auto it = m_ast->cbegin(); it != m_ast->cend(); ++it) {
    result = execute_node(*env, *it);
  }
  return result;
}

Value Interpreter::intrinsic_print(Value args[], unsigned num_args, const Location &loc, Interpreter *interp) {
  if (num_args != 1) EvaluationError::raise(loc, "Intrinsic print function expected 1 argument");
  std::cout << args[0].as_str();
  return Value(0);
}

Value Interpreter::intrinsic_println(Value args[], unsigned num_args,  const Location &loc, Interpreter *interp){
  if (num_args != 1) EvaluationError::raise(loc, "Intrinsic println expected 1 argument");
  std::cout << args[0].as_str() << std::endl;
  return Value(0);
}

Value Interpreter::intrinsic_readint(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
  if (num_args != 0) EvaluationError::raise(loc, "Intrinsic readint function expected 0 arguments");
  int i;
  std::cin >> i;
  return Value(i);
}

// recursively execute node based on its type, returning Value object to represent results
Value Interpreter::execute_node(Environment& env, Node* node) {
  int node_tag = node->get_tag();
  switch (node_tag) {
    // arithmetic operators
    case AST_ADD: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() + r.get_ival());
    }
    case AST_SUB: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() - r.get_ival());
    }
    case AST_MULTIPLY: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() * r.get_ival());
    }
    case AST_DIVIDE: {
      Value numerator = execute_node(env, node->get_kid(0)), denominator = execute_node(env, node->get_kid(1));
      if (!numerator.is_numeric() || !denominator.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      if (denominator.get_ival() == 0) EvaluationError::raise(node->get_loc(),"Division by zero");
      return Value(numerator.get_ival()/denominator.get_ival());
    }
    case AST_VARREF:
      return env.get_var(node->get_str());
    case AST_INT_LITERAL:
      return Value(std::stoi(node->get_str()));
    case AST_UNIT: {
      Value res;
      for (auto it = node->cbegin(); it != node->cend(); ++it) {
        res = execute_node(env, *it);
      }
      return res;
    }
    case AST_STATEMENT:
      return execute_node(env, node->get_kid(0));
    case AST_VARDEF:
      return env.create_var(node->get_kid(0)->get_str());
    // logical operators
    case AST_EQUAL:
      return env.set_var(node->get_kid(0)->get_str(), execute_node(env, node->get_kid(1)).get_ival());
    case AST_OR: {
      Value l = execute_node(env, node->get_kid(0));
      if (!l.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      if (l.get_ival()) return Value(1);
      Value r = execute_node(env, node->get_kid(1));
      if (!r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() || r.get_ival());
    }
    case AST_AND: {
      Value l = execute_node(env, node->get_kid(0));
      if (!l.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      if (!l.get_ival()) return Value(0);
      Value r = execute_node(env, node->get_kid(1));
      if (!r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() && r.get_ival());
    }
    // relational operators
    case AST_LESSER: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() < r.get_ival());
    }
    case AST_LESSER_EQUAL: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() <= r.get_ival());
    }
    case AST_GREATER: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() > r.get_ival());
    }
    case AST_GREATER_EQUAL: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() >= r.get_ival());
    }
    case AST_EQUAL_EQUAL: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() == r.get_ival());
    }
    case AST_NOT_EQUAL: {
      Value l = execute_node(env, node->get_kid(0)), r = execute_node(env, node->get_kid(1));
      if (!l.is_numeric() || !r.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      return Value(l.get_ival() != r.get_ival());
    }
    // control
    case AST_IF: {
      Value if_cond = execute_node(env,node->get_kid(0));
      if (!if_cond.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      if (if_cond.get_ival() != 0) {
        execute_node(env, node->get_kid(1));
      } else if (node->get_num_kids() == 3) { // there's an else
        execute_node(env, node->get_kid(2)->get_kid(0));
      }
      return Value(0);
    }
    case AST_WHILE: {
      Value while_cond = execute_node(env,node->get_kid(0));
      if (!while_cond.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      while (while_cond.get_ival() != 0) {
        execute_node(env, node->get_kid(1));
        while_cond = execute_node(env, node->get_kid(0));
        if (!while_cond.is_numeric()) EvaluationError::raise(node->get_loc(), "Use of non-numeric value");
      }
      return Value(0);
    }
    case AST_STMTS: {
      Environment* new_env = new Environment(&env); // block scope
      Value res;
      for (auto it = node->cbegin(); it != node->cend(); ++it) {
        Node* child_node = *it;
        res = execute_node(*new_env, child_node);
      }
      delete new_env;
      return res;
    }
    case AST_FUNC:
      return Value(0);
    case AST_FUNC_CALL: {
      std::string func_name = node->get_kid(0)->get_str();
      if (node->get_num_kids() > 1) { // func has args
        int arg_ct = node->get_kid(1)->get_num_kids();
        Value args[arg_ct];
        for (int i = 0; i < arg_ct; i++) {
          args[i] = execute_node(env, node->get_kid(1)->get_kid(i));
        }
        return env.function_call(func_name, args, arg_ct, node->get_loc(), *this);
      }
    }
    default:
      EvaluationError::raise(node->get_loc(),"Unrecognized node type");
  }
}