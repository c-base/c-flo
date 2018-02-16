import msgflo
from qhue import Bridge
from rgbxy import Converter

class farbhue(msgflo.Participant):
    def __init__(self, role):
        d = {
          'component': 'c-flo/farbhue',
          'label': 'Transmit Farbgeber colors to Philips Hue lights',
          'icon': 'lightbulb-o',
          'inports': [
            { 'id': 'in', 'type': 'object'},
          ],
          'outports': [
            { 'id': 'out', 'type': 'object' },
          ],
        }
        msgflo.Participant.__init__(self, d, role)

    def process(self, inport, msg):
        hueUser = "psyw1FycddUaRxU2oDyNoPja6jtWlZssJJ8Z9R6V"
        bridge = Bridge("10.0.0.159", hueUser)
        converter = Converter()
        # Convert farbgeber to Hue xy
        xy = converter.rgb_to_xy(msg.data['v1'][0], msg.data['v1'][1], msg.data['v1'][2])
        # Send to all lights
        for lightId in bridge.lights():
            bridge.lights[lightId].state(xy=xy)
        # Send light status out
        self.send('out', bridge.lights())
        self.ack(msg)

if __name__ == '__main__':
    msgflo.main(farbhue)
