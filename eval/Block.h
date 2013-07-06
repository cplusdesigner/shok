#ifndef _Block_h_
#define _Block_h_

/* Block construct
 *
 * The Token (construction-time) does not have enough information to know if
 * this is a code block (list of statements) or an expression block (single
 * expression).  We have to wait until complete()-time to determine which.
 */

#include "Brace.h"
#include "ExpressionBlock.h"
#include "Log.h"
#include "Token.h"
//#include "Statement.h"
#include "Variable.h"

#include <set>

namespace eval {

class Block : public Brace {
public:
  Block(Log&, const Token&);
  virtual ~Block();

  virtual void complete();
  virtual void evaluate();
  virtual std::string cmdText() const;

  bool isInScope(Variable*) const;
  void addVariable(Variable*);

private:
  ExpressionBlock* m_expBlock;
  std::set<Variable*> m_variables;
  //std::vector<Statement> m_statements;
};

};

#endif // _Block_h_
