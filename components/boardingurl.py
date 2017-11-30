#!/usr/bin/env python3

import msgflo
import urllib.parse

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
    greeting = 'hallo %s, willkommen an bord!' % msg.data['user']
    url = 'http://c-flo.cbrp3.c-base.org/visual-paging/?%s' % urllib.parse.quote(greeting.encode('utf-8'))
    self.send('out', url)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(BoardingUrl)
