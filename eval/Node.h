#ifndef _Node_h_
#define _Node_h_

/* AST node
 *
 * Use the MakeNode factory constructor to create the appropriate
 * subclass for a given AST token.  At construction, not all of its
 * members will be set to their correct values.  First you must add
 * all its child nodes via addChild(), then call complete().  This
 * will run some validation checks on the node, complete() its
 * children, and mark it as ready for use.  If the node represents a
 * brace/bracket/etc. that needs to be matched
 */

#include "Log.h"
#include "Token.h"

#include <string>
#include <deque>

namespace eval {

struct Node {

  friend class AST;

  static Node* MakeNode(Log&, const Token&);

  Node(Log& log, const Token& token);
  virtual ~Node();

  void addChild(Node* child);
  bool isComplete() const;

  virtual std::string print() const;
  virtual void complete() = 0;
  virtual void evaluate() = 0;
  virtual std::string cmdText() const;

protected:
  typedef std::deque<Node*> child_vec;
  typedef child_vec::const_iterator child_iter;

  // Set by constructor
  Log& log;
  std::string name;
  std::string value;
  bool m_isComplete;
  // Set by AST prior to completion
  unsigned int depth;
  Node* parent;
  child_vec children;
};

struct RootNode : public Node {
  RootNode(Log& log);

  virtual void complete();
  virtual void evaluate();
};

};

#endif // _Node_h_
