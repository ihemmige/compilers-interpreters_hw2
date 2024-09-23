#include "environment.h"


Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() {
}

Value Environment::get_var(std::string var) {
  if (m_lookup_table.find(var) == m_lookup_table.end()) {
    return m_parent->get_var(var);
  }
  return m_lookup_table[var];
}

Value Environment::set_var(std::string var, int value) {
  if (m_lookup_table.find(var) == m_lookup_table.end()) {
    return m_parent->set_var(var, value);
  }
  m_lookup_table[var] = Value(value);
  return m_lookup_table[var];
}

Value Environment::create_var(std::string var) {
  m_lookup_table[var] = Value(0);
  return m_lookup_table[var];
}

Value Environment::bind(std::string func_name, Value func) {
  if (func.get_kind() != VALUE_INTRINSIC_FN && func.get_kind() != VALUE_FUNCTION) {
    RuntimeError::raise("Tried to bind an object that isn't a function.");
  }
  m_lookup_table[func_name] = func;
  return m_lookup_table[func_name];
}

Value Environment::function_call(std::string func_name, Value args[], int num_args, const Location &location, Interpreter& interpreter) {
  if (m_lookup_table.find(func_name) == m_lookup_table.end()) return m_parent->function_call(func_name, args, num_args, location, interpreter);

  Value function = m_lookup_table[func_name];

  if (function.get_kind() != VALUE_INTRINSIC_FN && function.get_kind() != VALUE_FUNCTION) RuntimeError::raise("Tried to bind an object that isn't a function.");
  
  if (function.get_kind() == VALUE_INTRINSIC_FN) {
    IntrinsicFn intrin_func = function.get_intrinsic_fn();
    return Value(intrin_func(args, num_args, location,  &interpreter));
  }
  return Value(0);
}