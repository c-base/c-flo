#!/usr/bin/env python3

import msgflo
import urllib.request
import json
import datetime
import dateutil.parser
from pytz import timezone

def get_next_launch():
    url = "https://launchlibrary.net/1.3/launch/next/1"
    response = urllib.request.urlopen(url)
    data = json.loads(response.read().decode('utf-8'))
    return data["launches"][0]

def is_launch_on_date(launch, date):
    net = dateutil.parser.parse(launch["net"])
    if net.date() == date.date():
        return True
    return False

def is_launch_in_minutes(launch, window_open, minutes):
    net = dateutil.parser.parse(launch["net"])
    window_close = window_open + datetime.timedelta(minutes = minutes)
    if net < window_open:
        return False
    if net >= window_close:
        return False
    return True

def minutes_to_launch(launch, now):
    timeToLaunch = dateutil.parser.parse(launch["net"]) - now 
    return round(timeToLaunch.total_seconds() / 60)

def launch_to_string(launch, now):
    minutes = minutes_to_launch(launch, now)
    if (len(launch["missions"]) > 0):
        return '%s "%s" is launching from %s in %d minutes' % (launch["rocket"]["name"], launch["missions"][0]["name"], launch["location"]["name"], minutes)
    return '%s is launching from %s in %d minutes' % (launch["rocket"]["name"], launch["location"]["name"], minutes)

class LaunchAnnouncer(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/LaunchAnnouncer',
      'label': 'Generate an announcement for space launches',
      'icon': 'rocket',
      'inports': [
        { 'id': 'in', 'type': 'bang' },
      ],
      'outports': [
        { 'id': 'out', 'type': 'string' },
        { 'id': 'skipped', 'type': 'string' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    next_launch = get_next_launch()
    now = datetime.datetime.now(timezone('Europe/Berlin'))
    # We only generate announcements on certain times
    times = [5, 10, 15, 30, 60]
    minutes = minutes_to_launch(next_launch, now)
    if minutes in times:
      self.send('out', launch_to_string(next_launch, now))
    else:
      self.send('skipped', launch_to_string(next_launch, now))

    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(LaunchAnnouncer)
