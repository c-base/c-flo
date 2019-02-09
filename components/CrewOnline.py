#!/usr/bin/env python3

import msgflo
import requests

class CrewOnline(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/CrewOnline',
      'label': 'Who is connected to c-base crew network',
      'icon': 'child',
      'inports': [
          { 'id': 'in', 'type': 'bang'}
      ],
      'outports': [
          { 'id': 'out', 'type': 'array'},
          { 'id': 'bar', 'type': 'boolean', 'queue': 'bar/state'}
      ]
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    url = "https://c-beam.cbrp3.c-base.org/mechblast_json"
    response = requests.get(url, verify=False)
    data = response.json()
    self.send('out', data["userlist"])
    if data["barstatus"]:
      self.send('bar', "open")
    else:
      self.send('bar', "closed")
    self.ack(msg)
