#!/usr/bin/env python3

import msgflo

class BoardingUrl(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/boardingurl',
      'label': 'Generate greeting URL for crew members that are boarding',
      'icon': 'check',
      'inports': [
        { 'id': 'in', 'type': 'object' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'string' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    url = 'https://c-beam.cbrp3.c-base.org/welcome/%s' % msg.data['user']
    self.send('out', url)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(BoardingUrl)
