#ifndef _Function_h_
#define _Function_h_

/* Function objects, including methods */

#include "Log.h"
#include "Type.h"
#include "Variable.h"

#include <string>

namespace eval {

class Function {
public:
  Function();
  ~Function();

private:
  /*
  struct Arg {
    std::string name;
    Type type;
    Object* defaultValue;
  };
  */

  //vector<Arg> args;
  //Type returnType;
};

};

#endif // _Function_h_
