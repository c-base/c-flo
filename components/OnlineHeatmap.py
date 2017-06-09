#!/usr/bin/env python
import msgflo

class OnlineHeatmap(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/OnlineHeatmap',
      'label': 'Show through colours how many members are online',
      'icon': 'diamond',
      'inports': [
      	{ 'id': 'in', 'type': 'array'}
      ],
      'outports': [
      	{ 'id': 'out', 'type': 'object'}
      ]
    }
    msgflo.Participant.__init__(self, d, role)

  def generate_color(self, online):
    length = len(online)
    if length < 3:
      return [0,31,151]
    if length < 6:
      return [2,126,199]
    if length < 9:
      return [17,255,243]
    if length < 12:
      return [29,255,0]
    if length < 15:
      return [255,251,0]
    if length < 18:
      return [253,152,0]
    if length < 21:
      return [250,21,0]
   
    return [128,0,0]
    
    
  def process(self, inport, msg):
    colors = dict()
    colors['b'] = self.generate_color(msg.data)
    colors['v1'] = self.generate_color(msg.data)
    colors['v2'] = self.generate_color(msg.data)
    colors['v3'] = self.generate_color(msg.data)
    colors['v4'] = self.generate_color(msg.data)
    colors['c'] = self.generate_color(msg.data)
  
  
    self.send('out', colors)
    self.ack(msg)