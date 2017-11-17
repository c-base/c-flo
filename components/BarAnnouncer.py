#!/usr/bin/env python3

import msgflo

class BarAnnouncer(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/BarAnnouncer',
      'label': 'Generate an announcement when bar opens or closes',
      'icon': 'commenting',
      'inports': [
        { 'id': 'in', 'type': 'string' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'string' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if msg.data.find('bar opening') != -1:
      self.send('out', 'Bar is opening')
    if msg.data.find('bar closing') != -1:
      self.send('out', 'Bar is closing')
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(BarAnnouncer)
