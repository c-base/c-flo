#!/usr/bin/env python
import sys, os, json, logging
sys.path.append(os.path.abspath("."))
import gevent
import msgflo
from itertools import cycle

log = logging.getLogger(__name__)

DISCO_COLORS=cycle([(0xFF, 0x00, 0x00), (0, 102, 255), (255, 0, 51), (0xff, 0xff, 0x0)])
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
    'dmx-1-40/rgb',
    'dmx-1-1/rgb',
    'dmx-1-55/rgb',
    'dmx-1-49/rgb',
    'dmx-1-43/rgb',
    'dmx-1-31/rgb',
]

# RGB_NAMES = ['dmx-1-1/rgb',]

class DiscoAnimation(msgflo.Participant):
    def __init__(self, role):
        d = {
          'component': 'c-flo/DiscoAnimation',
          'label': 'Disco animation inspired by Saturday Night Fever',
          'icon': 'fire',
          'inports': [
            { 'id': 'is_enabled', 'type': 'boolean'},
            { 'id': 'channels', 'type': 'object' },
            { 'id': 'dmx', 'type': 'object' },
          ],
          'outports': [
            { 'id': 'running', 'type': 'boolean'},
            { 'id': 'animation', 'type': 'object' },
            { 'id': 'colours', 'type': 'array'},
          ],
        }
        self.is_enabled = False
        self.tick = 0
        self.original_channels = None
        msgflo.Participant.__init__(self, d, role)

    def loop(self):
        while self.is_enabled == True:
            print("loop")
            self.tick += 1
            for i in range(self.tick % 3):
                next(DISCO_COLORS)
            channels = []    
            for i in RGB_NAMES:
                color = next(DISCO_COLORS)
                channels.append({'channel_id': '%s/r' % i, 'value': color[0]})
                channels.append({'channel_id': '%s/g' % i, 'value': color[1]})
                channels.append({'channel_id': '%s/b' % i, 'value': color[2]})
            color = next(DISCO_COLORS)
            self.send('animation', channels)
            self.send('colours', list(color))
            print("Channels", channels)
            gevent.sleep(0.8)

    def process(self, inport, msg):
        log.info("Process here, inport is %s" % inport)
        if inport == 'dmx':
            # Allow others to write DMX only when we're not doing disco animation
            if self.is_enabled == False:
                self.send('animation', msg.data)
            self.ack(msg)
        if inport == 'channels': 
            # Store the current state of the DMX lights to be restored later.
            if self.is_enabled == False:
                self.original_channels = []
                self.original_channels = msg.data
                self.ack(msg)
        elif inport == 'is_enabled':
            self.send('running', msg.data)
            if msg.data == False:
                if self.is_enabled == True:
                    self.is_enabled = msg.data
                    # Send the last know good state when we are done with the animation
                    if self.original_channels != None:
                        self.send('animation', self.original_channels)
                    self.ack(msg)
            if msg.data == True:
                if self.is_enabled == True:
                    self.ack(msg)
                else:
                    self.is_enabled = msg.data
                    self.ack(msg)
                    gevent.Greenlet.spawn(self.loop)
        else:
            self.ack(msg)


if __name__ == '__main__':
    msgflo.main(DiscoAnimation)
