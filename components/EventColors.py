#!/usr/bin/env python3
import msgflo

def getCryptoParty(palette):
  #CryptoParty colors: purple(217/0/255 | d900ff) && pink(255/0/152 | ff0098)
  cryptoPartyPurple = [217, 0, 255]
  cryptoPartyPink   = [255, 0, 152]
  return {
    'b': cryptoPartyPink, # base color, usually not utilized
    'c': cryptoPartyPurple, # columns
    'v1': cryptoPartyPink, # ceiling1
    'v2': cryptoPartyPink, # ceiling2
    'v3': cryptoPartyPurple, # wall? in `dmx` wall is randomly chosen from v1-v4
    'v4': cryptoPartyPurple, # gate
  }

def handleEvents(events, palette):
  if len(events) == 0:
    # farbgeber when there are no events
    return palette
  for event in events:
    if event['summary'].lower() == 'cryptoparty':
      return getCryptoParty(palette)
  # Default to farbgeber
  return palette

class EventColors(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/EventColors',
      'label': 'Produce color schemes for events',
      'icon': 'palette',
      'inports': [
        { 'id': 'current', 'type': 'array'},
        { 'id': 'colors', 'type': 'object' },
      ],
      'outports': [
        { 'id': 'palette', 'type': 'object' },
      ],
    }
    self.palette = None
    self.current_events = []
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if inport == 'current':
      self.current_events = msg.data
      if self.palette:
        self.send('palette', handleEvents(msg.data, self.palette))
      self.ack(msg)
      return
    if inport == 'colors':
      self.palette = msg.data
      self.send('palette', handleEvents(self.current_events, msg.data))
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(EventColors)
