#!/usr/bin/env python
import sys, os, json, logging
sys.path.append(os.path.abspath("."))
import gevent
import msgflo
from datetime import datetime
from datetime import timedelta

class NetworkBars(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-base/NetworkBars',
      'label': 'Create bars from traffic stats',
      'icon': 'check',
      'inports': [
        { 'id': 'traffic_in', 'type': 'object' },
      ],
      'outports': [
        { 'id': 'dmx_out', 'type': 'object' },
      ],
    }
    self.last_val= {'rx': -1, 'tx': -1}
    self.last_time = None
    self.max_bits = 58080 # 10 mbit/s
    msgflo.Participant.__init__(self, d, role)
    
  def diff(self, msg, direction):
    curr_val = 0
    curr_time = datetime.now()
    for iface in msg.data['interfaces']:
      curr_val += int(iface[direction])
    
    if self.last_val[direction] == -1 or self.last_time == None:
      self.last_val[direction] = curr_val
      self.last_time = curr_time
      return 0.0
    time_passed = curr_time - self.last_time
    # division by zero
    if time_passed.seconds < 1: 
      time_passed = 1
    the_rate = float(curr_val - self.last_val[direction]) / float(time_passed.seconds)
  
  def rate(self, msg, direction):
    curr_val = 0
    curr_time = datetime.now()
    for iface in msg.data['interfaces']:
      curr_val += int(iface[direction])
    if self.last_time == None:
      time_passed = 60 # seconds
    else:
      time_passed = (curr_time - self.last_time).seconds
    the_rate = curr_val / float(time_passed)
    return the_rate
    
  def process(self, inport, msg):
    self.ack(msg)
    lights = []
    num = int((self.rate(msg, 'rx')/float(self.max_bits)) * 8)
    for i in range(num):
      lights.append({"channel_id": "dmx-1-159/rgb%d/r" % (i+1), "value": 255})
      lights.append({"channel_id": "dmx-1-159/rgb%d/g" % (i+1), "value": 0})
      lights.append({"channel_id": "dmx-1-159/rgb%d/b" % (i+1), "value": 0})
    for i in range(8 - num):
      lights.append({"channel_id": "dmx-1-159/rgb%d/r" % (num+i+1), "value": 0})
      lights.append({"channel_id": "dmx-1-159/rgb%d/g" % (num+i+1), "value": 0})
      lights.append({"channel_id": "dmx-1-159/rgb%d/b" % (num+i+1), "value": 0})
    self.send('dmx_out', lights)
    


def main():
  waiter = gevent.event.AsyncResult()
  role = sys.argv[1] if len(sys.argv) > 1 else 'repeat'
  repeater = Repeat(role)
  engine = msgflo.run(repeater, done_cb=waiter.set)

  print "Repeat running on %s" % (engine.broker_url)
  sys.stdout.flush()
  waiter.wait()
  print "Shutdown"
  sys.stdout.flush()

if __name__ == '__main__':
  logging.basicConfig()
  main()
