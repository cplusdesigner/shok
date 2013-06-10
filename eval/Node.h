#ifndef _Node_h_
#define _Node_h_

/* AST node */

#include "Code.h"
#include "Log.h"
#include "Token.h"

#include <string>
#include <deque>

namespace eval {

class Code;

struct Node {
  Node(Log& log, const Token& token);
  virtual ~Node();

  std::string print() const;
  void addChild(Node* child);
  void evaluate();

  Log& log;
  bool completed;
  unsigned int depth;
  std::string name;
  std::string value;
  Node* parent;
  typedef std::deque<Node*> child_vec;
  typedef std::deque<Node*>::const_iterator child_vec_iter;
  child_vec children;
};

};

#endif // _Node_h_
