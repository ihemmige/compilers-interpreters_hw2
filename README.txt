This assignment consisted of three parts, updates to the Lexer, updates to the Parser, and the Interpreter. 

The updates to the Lexer were relatively straightforward. I added in the new keywords, as well as the comma and brace characters. To accomodate 
these, I also added in a new token types. 

For the Parser, the approach was not much different from assignment 1, generally following the grammars defined. Depending on 
the specific non-terminal symbol being expanded, I checked tokens ahead with peek() to determine which grammar rule to follow. 
When the grammar consisted of further non-terminal symbols, I made the corresponding parse call, and the results of that parse
could be appended as a child to the current Node.

For the interpreter, the first step was the semantic analyzer. I had to make changes to the Environment class to allow for child environments
inheriting from a parent environment. This also meant rewriting the recursive semantic analyzer that ensured that all identifiers were defined
before their use. I added in the intrinsic functions as directed in the assignment directions, and added a few more cases in the execute_node
function to handle control flow and function calls.