#!/usr/bin/env python2

import msgflo

class HighPower(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/HighPower',
      'label': 'Announce when c-base is using a lot of power',
      'icon': 'check',
      'inports': [
        { 'id': 'in', 'type': 'int' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'string' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if (msg.data < 9000):
      self.ack(msg)
      return
    announcement = 'Power consumption over 9000'
    self.send('out', announcement)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(HighPower)
