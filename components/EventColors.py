#!/usr/bin/env python3
import gevent
import msgflo

def hasCryptoParty(events):
  for event in events:
    if event['summary'].lower() == 'cryptoparty':
      return True
  return False

#CryptoParty colors: purple(217/0/255 | d900ff) && pink(255/0/152 | ff0098)
cryptoPartyPurple = [217, 0, 255]
cryptoPartyPink   = [255, 0, 152]

def getColorsForDmx():
  return {
    'b': cryptoPartyPink, # base color, usually not utilized
    'c': cryptoPartyPurple, # columns
    'v1': cryptoPartyPink, # ceiling1
    'v2': cryptoPartyPink, # ceiling2
    'v3': cryptoPartyPurple, # wall? in `dmx` wall is randomly chosen from v1-v4
    'v4': cryptoPartyPurple, # gate
  }


class DetectCryptoParty(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/DetectCryptoParty',
      'label': 'Detect a CryptoParty event',
      'icon': 'unlock-alt ',
      'inports': [
        { 'id': 'current', 'type': 'array'}
      ],
      'outports': [
        { 'id': 'palette', 'type': 'object' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if len(msg.data) and hasCryptoParty(msg.data):
      self.send('palette', getColorsForDmx())
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(DetectCryptoParty)
