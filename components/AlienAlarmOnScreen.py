#!/usr/bin/env python3

import msgflo

class AlienAlarmOnScreen(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/AlienAlarmOnScreen',
      'label': 'Show visual Alien Alarm when needed',
      'icon': 'bug',
      'inports': [
        { 'id': 'in', 'type': 'boolean' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'string' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if msg.data:
      print("Alien Alarm!")
      self.send('out', 'http://c-flo.cbrp3.c-base.org/alien-alarm/')
    else:
      print("Alarm over")
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(AlienAlarmOnScreen)
