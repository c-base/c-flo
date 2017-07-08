#!/usr/bin/env python
import msgflo

class IgniteMainhall(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/IgniteMainhall',
      'label': 'Send true if both inputs are true',
      'icon': 'code-fork',
      'inports': [
        { 'id': 'ignite', 'type': 'boolean' },
        { 'id': 'start', 'type': 'boolean' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'boolean' },
      ],
    }
    self.igniteState = False
    self.startState = False
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if inport == "ignite":
      self.igniteState = msg.data
    if inport == "start":
      self.startState = msg.data
    if self.igniteState and self.startState:
      self.send("out", True)
    else:
      self.send("out", False)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(IgniteMainhall)
