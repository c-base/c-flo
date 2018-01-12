import msgflo
import random

FARB_VARIANTS = [
    'v1',
    'v2',
    'v3',
    'v4',
]

def randomVariant():
    return random.choice(FARB_VARIANTS)

RGB_COLUMNS = [
    'dmx-1-7/rgb',
    'dmx-1-1/rgb',
    'dmx-1-13/rgb',
    'dmx-1-19/rgb',
    'dmx-1-25/rgb',
    'dmx-1-31/rgb',
]

RGB_GATE = [
    # c-gate
    'dmx-1-55/rgb',
    'dmx-1-60/rgb',
]

RGB_WALL = [
    # Stage wall 1
    'dmx-1-159/rgb1',
    'dmx-1-159/rgb3',
    'dmx-1-159/rgb2',
    'dmx-1-159/rgb5',
    'dmx-1-159/rgb4',
    'dmx-1-159/rgb7',
    'dmx-1-159/rgb6',
    'dmx-1-159/rgb8',
    # Stage wall 2
    'dmx-1-185/rgb1',
    'dmx-1-185/rgb2',
    'dmx-1-185/rgb3',
    'dmx-1-185/rgb4',
    'dmx-1-185/rgb5',
    'dmx-1-185/rgb6',
    'dmx-1-185/rgb7',
    'dmx-1-185/rgb8',
]

RGB_CEILING = [
    # Ceiling lights
    'dmx-1-37/rgb',
    'dmx-1-43/rgb',
    'dmx-1-49/rgb',
]
RGB_CEILING2 = [
    # Ceiling lights
    'dmx-1-40/rgb',
    'dmx-1-46/rgb',
    'dmx-1-52/rgb',
]

class farbdmx(msgflo.Participant):
    def __init__(self, role):
        d = {
          'component': 'c-flo/farbdmx',
          'label': 'Convert Farbgeber colors to DMX',
          'icon': 'filter',
          'inports': [
            { 'id': 'in', 'type': 'object'},
          ],
          'outports': [
            { 'id': 'out', 'type': 'object' },
          ],
        }
        msgflo.Participant.__init__(self, d, role)

    def process(self, inport, msg):
        channels = []
        for i in RGB_COLUMNS:
            channels.append({'channel_id': '%s/r' % i, 'value': msg.data['c'][0]})
            channels.append({'channel_id': '%s/g' % i, 'value': msg.data['c'][1]})
            channels.append({'channel_id': '%s/b' % i, 'value': msg.data['c'][2]})
        for i in RGB_CEILING:
            channels.append({'channel_id': '%s/r' % i, 'value': msg.data['v1'][0]})
            channels.append({'channel_id': '%s/g' % i, 'value': msg.data['v1'][1]})
            channels.append({'channel_id': '%s/b' % i, 'value': msg.data['v1'][2]})
        for i in RGB_CEILING2:
            channels.append({'channel_id': '%s/r' % i, 'value': msg.data['v2'][0]})
            channels.append({'channel_id': '%s/g' % i, 'value': msg.data['v2'][1]})
            channels.append({'channel_id': '%s/b' % i, 'value': msg.data['v2'][2]})
        for i in RGB_WALL:
            key = randomVariant()
            channels.append({'channel_id': '%s/r' % i, 'value': msg.data[key][0]})
            channels.append({'channel_id': '%s/g' % i, 'value': msg.data[key][1]})
            channels.append({'channel_id': '%s/b' % i, 'value': msg.data[key][2]})
        for i in RGB_GATE:
            channels.append({'channel_id': '%s/r' % i, 'value': msg.data['v4'][0]})
            channels.append({'channel_id': '%s/g' % i, 'value': msg.data['v4'][1]})
            channels.append({'channel_id': '%s/b' % i, 'value': msg.data['v4'][2]})
        self.send('out', channels)
        self.ack(msg)

if __name__ == '__main__':
    msgflo.main(farbdmx)
