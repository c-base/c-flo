#!/usr/bin/env python
import sys, os, json, logging
sys.path.append(os.path.abspath("."))
import gevent
import msgflo
from itertools import cycle

log = logging.getLogger(__name__)

DISCO_COLORS=cycle([(222, 255,255), (0xfc, 0xda, 0x9f), (0xea, 0x99, 0xa4)])
# for i in a.keys(): s.add('/'.join(i.split('/')[:-1]))
RGB_NAMES = [
    'dmx-1-46/rgb',
    'dmx-1-60/rgb',
    'dmx-1-13/rgb',
    'dmx-1-185/rgb8',
    'dmx-1-185/rgb4',
    'dmx-1-185/rgb5',
    'dmx-1-185/rgb6',
    'dmx-1-185/rgb7',
    'dmx-1-185/rgb1',
    'dmx-1-185/rgb2',
    'dmx-1-185/rgb3',
    'dmx-1-159/rgb15',
    'dmx-1-159/rgb14',
    'dmx-1-159/rgb11',
    'dmx-1-159/rgb10',
    'dmx-1-159/rgb13',
    'dmx-1-159/rgb12',
    'dmx-1-7/rgb',
    'dmx-1-52/rgb',
    'dmx-1-211/rgb1',
    'dmx-1-211/rgb2',
    'dmx-1-211/rgb3',
    'dmx-1-211/rgb4',
    'dmx-1-211/rgb5',
    'dmx-1-211/rgb6',
    'dmx-1-211/rgb7',
    'dmx-1-211/rgb8',
    'dmx-1-25/rgb',
    'dmx-1-19/rgb',
    'dmx-1-159/rgb1',
    'dmx-1-159/rgb3',
    'dmx-1-159/rgb2',
    'dmx-1-159/rgb5',
    'dmx-1-159/rgb4',
    'dmx-1-159/rgb7',
    'dmx-1-159/rgb6',
    'dmx-1-159/rgb9',
    'dmx-1-159/rgb8',
    'dmx-1-37/rgb',
    'dmx-1-1/rgb',
    'dmx-1-40/rgb',
    'dmx-1-55/rgb',
    'dmx-1-49/rgb',
    'dmx-1-43/rgb',
    'dmx-1-31/rgb',
]


class DiscoAnimation(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/DiscoAnimation',
      'label': 'Disco animation inspired by Saturday Night Fever',
      'icon': 'fire',
      'inports': [
        { 'id': 'is_enabled', 'type': 'boolen'},
        { 'id': 'channels', 'type': 'object' },
      ],
      'outports': [
        { 'id': 'animation', 'type': 'object' },
      ],
    }
    self.is_enabled = False
    self.tick = 0
    self.original_channels = None
    msgflo.Participant.__init__(self, d, role)

  def loop(self):
      while self.is_enabled == True:
          print "loop"
          channels = []    
          for i in RGB_NAMES:
              color = next(DISCO_COLORS)
              channels.append({'channel_id': '%s/r', 'value': color[0]})
              channels.append({'channel_id': '%s/g', 'value': color[1]})
              channels.append({'channel_id': '%s/b', 'value': color[2]})
          self.send('animation', channels)
          print "Channels", channels
          gevent.sleep(1)

  def process(self, inport, msg):
    print("Process here, inport is %s" % inport)
    if inport == 'channels': 
        # Store the current state of the DMX lights to be restored later.
        if self.is_enabled == False:
            self.original_channels = msg.data
            self.ack(msg)
    elif inport == 'is_enabled':
        if msg.data == False:
            if self.is_enabled == True and self.original_channels != None:
                # Send the last know good state when we are done with the animation
                self.send('animation', self.original_channels)
                self.is_enabled = msg.data
                self.ack(msg)
        if msg.data == True:
            if self.is_enabled == True:
                self.ack(msg)
            else:
                self.is_enabled = msg.data
                self.ack(msg)
                gevent.Greenlet.spawn(self.loop)


if __name__ == '__main__':
  msgflo.main(DiscoAnimation)
