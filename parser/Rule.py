# Copyright (C) 2013 Michael Biggs.  See the LICENSE file at the top-level
# directory of this distribution and at http://lush-shell.org/copyright.html

import Parser

class Rule:
  def __init__(self,name,items):
    self.name = name
    self.items = items
    self.bad = False
    self.done = False

  def MakeParser(self,parent):
    return Parser.Parser(self, parent)

def IsRuleDone(x):
  if hasattr(x, 'done'):
    return x.done
  if isinstance(x, str):
    return False
  if isinstance(x, tuple):
    return IsRuleDone(x[0])
  if isinstance(x, list):
    return all([IsRuleDone(i) for i in x])
  raise Exception("Cannot check IsRuleDone from unknown rule type of '%s'" % x)

