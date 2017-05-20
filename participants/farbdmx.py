import msgflo

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
        for i in RGB_NAMES:
            channels.append({'channel_id': '%s/r' % i, 'value': msg.data['b'][0]})
            channels.append({'channel_id': '%s/g' % i, 'value': msg.data['b'][1]})
            channels.append({'channel_id': '%s/b' % i, 'value': msg.data['b'][2]})
        self.send('out', channels)
        self.ack(msg)

if __name__ == '__main__':
    msgflo.main(DiscoAnimation)
