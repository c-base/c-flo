#!/usr/bin/env python

import msgflo

RGB_CEILING = [
    # Ceiling lights
    'dmx-1-37/rgb',
    'dmx-1-43/rgb',
    'dmx-1-49/rgb',
    'dmx-1-40/rgb',
    'dmx-1-46/rgb',
    'dmx-1-52/rgb',
]

class MainhallTheme(msgflo.Participant):
    def __init__(self, role):
        d = {
          'component': 'c-flo/MainhallTheme',
          'label': 'Themes for the lights in the mainhall',
          'icon': 'fire',
          'inports': [
            { 'id': 'theme', 'type': 'string'},
            { 'id': 'channels', 'type': 'object' },
            { 'id': 'dmx', 'type': 'object' },
          ],
          'outports': [
            { 'id': 'out', 'type': 'object' },
          ],
        }
        self.theme = 'DMX'
        self.original_channels = None
        msgflo.Participant.__init__(self, d, role)


    def process(self, inport, msg):
        if inport == 'dmx':
            # Allow others to write DMX only when we're not doing a theme
            if self.theme == 'DMX':
                self.send('out', msg.data)
            self.ack(msg)
        if inport == 'channels':
            # Store the current state of the DMX lights to be restored later.
            if self.theme == 'DMX':
                self.original_channels = []
                for key, value in msg.data.items():
                    self.original_channels.append({'channel_id': key, 'value': value})
                self.ack(msg)
        elif inport == 'theme':
            if msg.data == 'DMX':
                if self.theme != 'DMX':
                    self.theme = 'DMX'
                    # Send the last know good state when we are done with the out
                    if self.original_channels != None:
                        self.send('out', self.original_channels)
                    self.ack(msg)
            else:
                if self.theme == msg.data
                    self.ack(msg)
                elif msg.data == 'WHITE':
                    for i in RGB_CEILING:
                        channels.append({'channel_id': '%s/r' % i, 'value': 255})
                        channels.append({'channel_id': '%s/g' % i, 'value': 255})
                        channels.append({'channel_id': '%s/b' % i, 'value': 255})
                    self.ack(msg)
                else:
        else:
            self.ack(msg)


if __name__ == '__main__':
    msgflo.main(MainHalltheme)
