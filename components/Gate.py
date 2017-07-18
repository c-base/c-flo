#!/usr/bin/env python2

import msgflo

class Gate(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/Gate',
      'label': 'Pass or block packets based on another input',
      'icon': 'step-forward',
      'inports': [
        { 'id': 'in', 'type': 'any' },
        { 'id': 'open', 'type': 'boolean' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'any' },
        { 'id': 'is_open', 'type': 'boolean' },
      ],
    }
    self.is_open = False
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if inport == 'open':
      self.is_open = msg.data
      self.send('is_open', self.is_open)
      self.ack(msg)
      return

    if self.is_open:
      self.send('out', msg.data)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(Gate)
