# Copyright (C) 2013 Michael Biggs.  See the LICENSE file at the top-level
# directory of this distribution and at http://lush-shell.org/copyright.html

from Parser import Parser, MakeParser
from Rule import Rule
from StarParser import Star
from copy import copy
import logging

# The SeqParser's rule is a list of Rules that will be matched in
# sequence; advancing only when an individual rule is done and turns
# bad if given another token.  The SeqParser is done anytime that its
# last rule is done.
#
# If a not-the-last rule is fed a token and it moves from done to
# not-bad, not-done: we start accumulating all the tokens we feed it.
# If finally it turns bad instead of turning done again, we advance not
# only with the current token, but first with all the history of tokens
# that we tried to feed to this intermediary step but were rejected.
#
# We emit results from an active parser only when it's confirmed and we move to
# the one past it.  The last rule's parses are not emit until finish()-time.
class SeqParser(Parser):
  def __init__(self,rule,parent):
    Parser.__init__(self, rule, parent)
    self.parsers = []   # parsers constructed for each of rule.items
    self.active = None  # active parser (top of self.parsers)
    self.pos = 0        # position in self.rule.items
    self.accum = []     # tokens we've consumed since last rule-done-time
    self.displays = []  # display for each parser
    self.firstlastparse = True  # has the last rule's parser emit its msg_start
    self.msg_parts = [] # start and end parts of each rule's msg
    self.lastfinish = ''  # finish() text from last time subparser was done
    logging.info("%s SeqParser initialized with rule: %s" % (self.name, self.rule))

  def parse(self,token):
    logging.debug("%s SeqParser at pos=%s attempting to parse token '%s'" % (self.name, self.pos, token))

    if self.pos >= len(self.rule.items):
      logging.debug("%s SeqParser moved beyond rule items; now bad" % self.name)
      self.bad = True
      self.done = False
    elif not self.active:
      logging.info("%s SeqParser making parser for %s at pos=%s" % (self.name, self.rule.items[self.pos], self.pos))
      item = self.rule.items[self.pos]
      msg = '%s'
      if type(item) is tuple:
        msg = item[1]
        item = item[0]
      self.msg_parts.append(msg.split('%s'))
      if len(self.msg_parts[-1]) > 2:
        raise Exception("%s SeqParser: has a msg part with > 2 parts" % self.name)
      self.parsers.append(MakeParser(item, self))
      self.displays.append('')
      logging.info("%s SeqParser now parsers=%s, len=%s" % (self.name, self.parsers, len(self.parsers)))
      self.active = self.parsers[-1]
      self.bad = self.active.bad
    if self.pos != len(self.parsers)-1:
      raise Exception("%s SeqParser: parsers (%s) vs. self.pos=%s mismatch; len=%s, lenrule=%s" % (self.name, self.parsers, self.pos, len(self.parsers), len(self.rule.items)))
    if self.bad:
      self.done = False
      raise Exception("%s SeqParser is bad; can't accept token '%s'" % (self.name, token))
    logging.info("%s SeqParser parsing token '%s'; active=%s,%s" % (self.name, token, self.pos, self.active.name))

    wasdone = self.active.done    # was the current active parser already done
    if wasdone:
      logging.debug("%s SeqParser was done, at '%s'" % (self.name, self.active.name))

    # Parse!
    newdisp = self.active.parse(token)
    logging.debug("%s SeqParser parsed token '%s'; now evaluating" % (self.name, token))
    self.displays[self.pos] += newdisp

    # if in last pos
    if self.pos == len(self.rule.items)-1:
      self.bad = self.bad or self.active.bad
      if self.bad:
        self.done = False
        return ''
      else:
        self.done = self.active.done
      if self.done:
        self.lastfinish = self.active.finish() + self.msg_parts[self.pos][1]
      logging.debug("%s SeqParser in last state; now bad=%s, done=%s" % (self.name, self.bad, self.done))
      return self.display(self.pos)

    if self.active.bad:
      if wasdone:
        # accept that state as having never parsed this token yet, and go
        # forward
        logging.info("%s SeqParser at token '%s' is force-advancing pos %s to %s" % (self.name, token, self.pos, self.pos+1))
        self.pos += 1
        self.active = None
        self.accum = []
        # we did not pass a %s, but we know we can't go back on this parser so
        # we can output it entirely as a part of the current %s.
        prevdisp = copy(self.lastfinish)
        self.lastfinish = ''
        disp = self.parse(token)
        if self.bad:
          return ''
        return prevdisp + disp
      elif self.accum:
        # Skip ahead to the next rule and run out our accum backlog
        self.pos += 1
        self.active = None
        self.accum.append(token)
        accum = copy(self.accum)
        self.accum = []
        disp = ''
        for a in accum:
          logging.info("%s SeqParser is force-advancing pos %s to %s with accumulated token '%s'" % (self.name, self.pos, self.pos+1, a))
          if not self.bad:
            disp += self.parse(a)
        if self.bad:
          return ''
        lastfinish = copy(self.lastfinish)
        self.lastfinish = ''
        return lastfinish + disp
      else:
        self.bad = True
        self.done = False
        logging.debug("%s SeqParser wasn't done, now bad" % self.name)
        return ''

    if self.active.done:
      self.lastfinish = self.displays[self.pos] + self.active.finish() + self.msg_parts[self.pos][1]
      if self.pos < len(self.rule.items)-1:
        self.lastfinish = self.msg_parts[self.pos][0] + self.lastfinish
      # stay here until we turn bad
      # if all upcoming rule items are Stars, then mark the whole Seq as done.
      logging.info("%s SeqParser active pos=%s went done; waiting until it turns bad before advancing" % (self.name, self.pos))
      self.accum = []
      nonstars = False
      for item in self.rule.items[self.pos+1:]:
        if type(item) is tuple:
        if not (isinstance(item, Star) or (type(item) is tuple and isinstance(item[0], Star))):
          nonstars = True
      if not nonstars:
        self.done = True
        logging.debug("%s SeqParser is earlydone" % self.name)
      return ''

    # we're ok, stay put for now
    logging.debug("%s SeqParser is staying put after '%s'" % (self.name, token))
    self.accum.append(token)
    return ''

  def display(self,pos):
    if pos > self.pos:
      raise Exception("cannot display pos > self.pos")
    disp = copy(self.displays[pos])
    self.displays[pos] = ''
    if pos == len(self.rule.items)-1:
      if self.firstlastparse:
        self.firstlastparse = False
        return self.msg_parts[pos][0] + disp
      else:
        return disp
    return self.msg_parts[pos][0] + disp + self.msg_parts[pos][1]

  def finish(self):
    if self.pos < len(self.rule.items)-1:
      return ''
    pdisp = self.displays[self.pos]
    fdisp = self.parsers[self.pos].finish()
    edisp = self.msg_parts[self.pos][1]
    #return pdisp + fdisp + edisp
    return self.lastfinish

class Seq(Rule):
  def MakeParser(self,parent):
    return SeqParser(self, parent)

