#ifndef _AST_h_
#define _AST_h_

/* Abstract Syntax Tree */

#include "EvalError.h"
#include "Log.h"
#include "Node.h"
#include "RootNode.h"
#include "Token.h"

#include <string>

namespace eval {

class AST {
public:
  AST(Log& log);
  ~AST();

  // Performs the ugly work of inserting an input "AST Token" into the AST.
  void insert(const Token& token);
  // Reset the AST to a correct state; may destroy some unevaluated code.
  void reset();
  // Analyze the AST and execute any appropriate, complete fragments of code
  void evaluate();
  // Pretty-print the contents of the AST to a string
  std::string print() const;

private:
  void init();
  void destroy();

  void runCode();

  Log& m_log;
  RootNode* m_root;
  Node* m_current;
};

};

#endif // _AST_h_
