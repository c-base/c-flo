#!/usr/bin/env python3
import msgflo
import urllib.request
import json

class GetLuftJetzt(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/GetLuftJetzt',
      'label': 'Get air quality measurement data',
      'icon': 'cloud',
      'inports': [
          { 'id': 'in', 'type': 'bang'},
          { 'id': 'station', 'type': 'string'}
      ],
      'outports': [
          { 'id': 'pm10', 'type': 'number'},
          { 'id': 'o3', 'type': 'number'},
          { 'id': 'no2', 'type': 'number'},
          { 'id': 'so2', 'type': 'number'},
          { 'id': 'co', 'type': 'number'},
      ]
    }
    self.station = None
    self.lastSeen = {
      'pm10': 0,
      'o3'  : 0,
      'no2' : 0,
      'so2' : 0,
      'co'  : 0,
    }
    self.measurements = [
      None,
      'pm10',
      'o3',
      'no2',
      'so2',
      'co'
    ]
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if inport == 'station':
      self.station = msg.data
      self.ack(msg)
      return
    if self.station == None:
      # No station configured, skip
      self.ack(msg)
      return
    url = 'https://luft.jetzt/api/%s' % urllib.parse.quote(self.station.encode('utf-8'))
    response = urllib.request.urlopen(url)
    data = json.loads(response.read())
    for measurement in data:
      port = self.measurements[measurement["data"]["pollutant"]]
      if port == None:
        continue
      if measurement["data"]["date_time"] < self.lastSeen[port]:
        # Already got this data
        continue
      self.send(port, measurement["data"]["value"])
      self.lastSeen[port] = measurement["data"]["date_time"]
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(GetLuftJetzt)
