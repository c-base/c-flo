#!/usr/bin/env python3
import gevent
import msgflo

class DetectABBA(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/DetectABBA',
      'label': 'Detect if ABBA',
      'icon': 'star-o',
      'inports': [
        { 'id': 'song', 'type': 'object' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'boolean' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    current_song = msg.data
    artist = current_song.get('artist', '')
    if artist.lower() == 'abba':
      self.send('out', True)
    else:
      self.send('out', False)
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(DetectABBA)
