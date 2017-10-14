#!/usr/bin/env python
import msgflo
from colour import Color

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
    # Get number of users online
    length = len(online)

    # Generate gradient
    cold = Color("blue")
    hot = Color("red")
    gradient = list(cold.range_to(hot, 22))
    
    if length >= 22:
        # More than this is always hot
        selected = gradient[-1]
    else:
        selected = gradient[length - 1]

    red = int(selected.get_red() * 255)
    green = int(selected.get_green() * 255)
    blue = int(selected.get_blue() * 255)
    return [red, green, blue]

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
