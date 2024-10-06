Assignment 2 consisted of three parts, updates to the Lexer, updates to the Parser, and the Interpreter. 

The updates to the Lexer were relatively straightforward. I added in the new keywords, as well as the comma and brace characters. To accomodate 
these, I also added in a few new token types. 

For the Parser, the approach was not much different from assignment 1, generally following the grammars defined. Depending on 
the specific non-terminal symbol being expanded, I checked tokens ahead with peek() to determine which grammar rule to follow. 
When the grammar consisted of further non-terminal symbols, I made the corresponding parse call, and the results of that parse
could be appended as a child to the current Node.

For the interpreter, the first step was the semantic analyzer. I had to make changes to the Environment class to allow for child environments
inheriting from a parent environment. This also meant rewriting the recursive semantic analyzer that ensured that all identifiers were defined
before their use. I added in the intrinsic functions as directed in the assignment directions, and added a few more cases in the execute_node
function to handle control flow and function calls.

For function calls, the main changes were in handling two types of AST nodes: AST_FUNC and AST_FUNC_CALL. When a user-defined function is
encountered, I had to read its param list and store that as part of the Function Val created. When a function was called (intrinsic or otherwise), 
the steps were to read any parameters in and create a new environment for the function. Then, based on whether if it's not an intrinsic function, 
another block environment is created and parameters are defined inside the block. Finally, the statement list that makes up the function
can be executed.