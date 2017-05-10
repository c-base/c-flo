#!/usr/bin/env python2

import msgflo

class ToggleLights(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-base/togglelights',
      'label': 'Switch traffic light states on every packet',
      'icon': 'toggle-on',
      'inports': [
        { 'id': 'in', 'type': 'bang' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'object' },
      ],
    }
    self.state = {
      'red': 1,
      'yellow': 0,
      'green': 0,
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if (self.state.red):
      self.state.red = 0
    else:
      self.state.red = 1
    if (self.state.green):
      self.state.green = 0
    else:
      self.state.green = 1
    self.send('out', self.state)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(ToggleLights)
