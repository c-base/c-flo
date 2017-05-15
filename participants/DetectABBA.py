#!/usr/bin/env python
import sys, os, json, logging
sys.path.append(os.path.abspath("."))
import gevent
import msgflo


class DetectABBA(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/DetectABBA',
      'label': 'Detect if ABBA',
      'inports': [
        { 'id': 'current_song_in', 'type': 'object' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'object' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    self.ack(msg)
    current_song = msg.data
    artist = current_song.get('artist', '')
    if artist.lower() == 'abba':
        self.send('out', True)
    else:
        self.send('out', False)


def main():
  waiter = gevent.event.AsyncResult()
  role = sys.argv[1] if len(sys.argv) > 1 else 'repeat'
  repeater = DetectABBA(role)
  engine = msgflo.run(repeater, done_cb=waiter.set)

  print "DetectABBA running on %s" % (engine.broker_url)
  sys.stdout.flush()
  waiter.wait()
  print "DetectABBA shutdown."
  sys.stdout.flush()

if __name__ == '__main__':
  logging.basicConfig()
  main()
