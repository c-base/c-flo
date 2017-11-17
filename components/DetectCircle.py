import msgflo

class DetectCircle(msgflo.Participant):
    def __init__(self, role):
        d = {
          'component': 'c-flo/DetectCircle',
          'label': 'Blink traffic lights red when there is a circle meeting',
          'icon': 'hand-paper-o',
          'inports': [
            { 'id': 'current', 'type': 'object'},
            { 'id': 'in', 'type': 'object' },
          ],
          'outports': [
            { 'id': 'out', 'type': 'object'},
          ],
        }
        self.is_circle = False
        self.is_animating = False
        self.previous_data = None
        msgflo.Participant.__init__(self, d, role)

    def process(self, inport, msg):
        if inport == 'in':
            self.previous_data = msg.data

            if self.is_circle:
                # Circle ongoing, drop packet
                self.ack(msg)
                return
            # No circle, let the light do other stuff
            self.send('out', msg.data)
            self.ack(msg)
            return
        if inport == 'current':
            if msg.data and msg.data['summary'] == 'circle':
                print("Circle event ongoing")
                self.is_circle = True
                lights = {
                        'red': 1,
                        'yellow': 0,
                        'green': 0,
                        }
                self.send('out', lights)
                self.ack(msg)
                return

            self.is_circle = False
            if self.previous_data:
                # Let lights go to pre-circle state
                self.send('out', self.previous_data)
                self.ack(msg)
                return
            # Otherwise go green
            lights = {
                    'red': 0,
                    'yellow': 0,
                    'green': 1,
                    }
            self.send('out', lights)
            self.ack(msg)
            return

if __name__ == '__main__':
    msgflo.main(DetectCircle)
