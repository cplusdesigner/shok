#include "Command.h"

#include "Block.h"
#include "CommandFragment.h"
#include "EvalError.h"
#include "Node.h"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <string>
#include <vector>
using std::string;
using std::vector;

using namespace eval;

void Command::setup() {
}

void Command::evaluate() {
  string cmd;
  for (Node::child_iter i = children.begin(); i != children.end(); ++i) {
    const Block* block = dynamic_cast<const Block*>(*i);
    if (block && block->isCodeBlock()) return;    // Code blocks aren't commands
    const CommandFragment* frag = dynamic_cast<const CommandFragment*>(*i);
    if (frag) {
      cmd += frag->cmdText();
    } else if (block) {
      cmd += block->cmdText();
    } else {
      throw EvalError("Command has an unsupported child: " + string(**i));
    }
  }
  log.info("RUNNING CMD: <" + cmd + ">");
  std::cout << "CMD:" << cmd << std::endl;
  log.info("DONE, NOW GET STATUS CODE");
  string line;
  std::getline(std::cin, line);
  int returnCode = boost::lexical_cast<int>(line);
  log.info("RETURN CODE: " + boost::lexical_cast<string>(returnCode));
}
