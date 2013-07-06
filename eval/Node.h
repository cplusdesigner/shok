#ifndef _Node_h_
#define _Node_h_

/* AST node
 *
 * Use the MakeNode factory constructor to create the appropriate
 * subclass for a given AST token.  At construction, not all of its
 * members will be set to their correct values.  First you must add
 * all its child nodes via addChild(), then call complete().  This
 * will run some validation checks on the node, complete() its
 * children, and mark it as ready for use.
 */

#include "Log.h"
#include "Token.h"

#include <string>
#include <deque>

namespace eval {

class Block;

class Node {
public:
  static Node* MakeNode(Log&, const Token&);

  // Insert a Node n into the tree at the provided "current" position.
  // Called by AST; returns the new current position, the original may
  // have been destroyed.
  static Node* insertNode(Node* current, Node*);

  virtual ~Node();

  // This will be called by insertNode() only on parent nodes.  It
  // should be the only thing that calls setupNode() and completeNode().
  void setupAndCompleteAsParent();
  // Setup properties that descend from the parent
  void setupNode();
  // Validate properties regarding the node's children
  void completeNode();
  // Reorder operator/expression trees for correct operator precedence
  void reorderOperators();
  // Not intended to be overridden by anything other than RootNode
  void analyzeNode();
  void evaluateNode();
  bool isEvaluated() { return m_isEvaluated; }

  virtual std::string print() const;
  virtual operator std::string() const;

protected:
  Node(Log&, const Token&);

  typedef std::deque<Node*> child_vec;
  typedef child_vec::const_iterator child_iter;
  typedef child_vec::iterator child_mod_iter;

  void addChild(Node* child);
  virtual void setup() {}
  virtual void complete() = 0;
  virtual void analyze() {}
  virtual void evaluate() = 0;

  // State flags
  bool m_isSetup;
  bool m_isComplete;
  bool m_isReordered;
  bool m_isAnalyzed;
  bool m_isEvaluated;

  // Set by constructor
  Log& log;
  std::string name;
  std::string value;
  // Set by addToken()
  Node* parent;
  child_vec children;
  // Set by setupNode(), parent-first
  Block* block;   // the most recent ancestor block (execution context)
  // Set by analyze(), children-first
  //Type type;
};

};

#endif // _Node_h_
