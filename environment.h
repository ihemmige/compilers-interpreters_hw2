#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"
#include <unordered_map>
#include "exceptions.h"

class Environment {
private:
  Environment *m_parent;
  std::unordered_map<std::string, Value> m_lookup_table; // map names to Values

  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();

  // functions to access, modify, and create variables
  Value get_var(std::string var);
  Value set_var(std::string var, int value);
  Value create_var(std::string var);
  Value bind(std::string func_name, Value func);
  Value function_call(std::string func_name, Value args[], int num_args, const Location &location, Interpreter& interpreter);
};

#endif // ENVIRONMENT_H
