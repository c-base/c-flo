#!/usr/bin/env python3

import msgflo
import urllib.request
import json

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
      	{ 'id': 'out', 'type': 'array'}
      ]
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    url = "https://c-beam.cbrp3.c-base.org/mechblast_json"
    response = urllib.request.urlopen(url)
    data = json.loads(response.read())
    self.send('out', data["userlist"])
    self.ack(msg)