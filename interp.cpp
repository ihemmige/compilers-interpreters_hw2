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
  for (auto it = parent->cbegin(); it != parent->cend(); ++it) {
    Node* child = *it;
    // if a var reference, and the var is not found
    if (child->get_tag() == AST_VARREF && var_set.find(child->get_str()) == var_set.end()) {
        SemanticError::raise(child->get_loc(), "Undefined variable %s", child->get_str().c_str());
    }
    check_vars(var_set, child);
  } 
}

void Interpreter::analyze() {
  std::unordered_set<std::string> var_set; // defined variables set
  for (auto it = m_ast->cbegin(); it != m_ast->cend(); ++it) {
    Node* child_node = *it;
    if (child_node->get_kid(0)->get_tag() == AST_VARDEF) { // if vardef, save the name
      var_set.insert(child_node->get_kid(0)->get_kid(0)->get_str());
    } else { // recursively check for varrefs 
      check_vars(var_set, child_node);
    }
  }
}

Value Interpreter::execute() {
  Value result;
  std::unique_ptr<Environment> env(new Environment());
  // execute each statement node in the tree
  for (auto it = m_ast->cbegin(); it != m_ast->cend(); ++it) {
    result = execute_node(*env, *it);
  }
  return result;
}

// recursively execute node based on its type, returning Value object to represent results
Value Interpreter::execute_node(Environment& env, Node* node) {
  int node_tag = node->get_tag();
  switch (node_tag) {
    // arithmetic operators
    case AST_ADD:
      return Value(execute_node(env, node->get_kid(0)).get_ival() + execute_node(env, node->get_kid(1)).get_ival());
    case AST_SUB:
      return Value(execute_node(env, node->get_kid(0)).get_ival() - execute_node(env, node->get_kid(1)).get_ival());
    case AST_MULTIPLY:
      return Value(execute_node(env, node->get_kid(0)).get_ival() * execute_node(env, node->get_kid(1)).get_ival());
    case AST_DIVIDE: {
      int denominator = execute_node(env, node->get_kid(1)).get_ival();
      if (denominator == 0) EvaluationError::raise(node->get_loc(),"Division by zero");
      return Value(execute_node(env, node->get_kid(0)).get_ival()/denominator);
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
    case AST_OR:
      return Value(execute_node(env, node->get_kid(0)).get_ival() || execute_node(env, node->get_kid(1)).get_ival());
    case AST_AND:
      return Value(execute_node(env, node->get_kid(0)).get_ival() && execute_node(env, node->get_kid(1)).get_ival());
    // relational operators
    case AST_LESSER:
      return Value(execute_node(env, node->get_kid(0)).get_ival() < execute_node(env, node->get_kid(1)).get_ival());
    case AST_LESSER_EQUAL:
      return Value(execute_node(env, node->get_kid(0)).get_ival() <= execute_node(env, node->get_kid(1)).get_ival());
    case AST_GREATER:
      return Value(execute_node(env, node->get_kid(0)).get_ival() > execute_node(env, node->get_kid(1)).get_ival());
    case AST_GREATER_EQUAL:
      return Value(execute_node(env, node->get_kid(0)).get_ival() >= execute_node(env, node->get_kid(1)).get_ival());
    case AST_EQUAL_EQUAL:
      return Value(execute_node(env, node->get_kid(0)).get_ival() == execute_node(env, node->get_kid(1)).get_ival());
    case AST_NOT_EQUAL:
      return Value(execute_node(env, node->get_kid(0)).get_ival() != execute_node(env, node->get_kid(1)).get_ival());
    // control
    case AST_IF: {
      Value if_cond = execute_node(env,node->get_kid(0));
      if (if_cond.get_ival() != 0) {
        execute_node(env, node->get_kid(1));
      } else if (node->get_num_kids() == 3) { // there's an else
        execute_node(env, node->get_kid(2)->get_kid(0));
      }
      return Value(0);
    }
    case AST_WHILE: {
      Value while_cond = execute_node(env,node->get_kid(0));
      while (while_cond.get_ival() != 0) {
        execute_node(env, node->get_kid(1));
        while_cond = execute_node(env,node->get_kid(0));
      }
      return Value(0);
    }
    case AST_STMTS: {
      Environment* new_env = new Environment(&env);
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
        const Location &location = node->get_loc();
        return env.function_call(func_name, args, arg_ct, location, *this);
      }
    }
    default:
      EvaluationError::raise(node->get_loc(),"Unrecognized node type");
  }
}