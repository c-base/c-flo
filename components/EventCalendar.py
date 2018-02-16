import urllib.request
import icalendar
import json
from datetime import datetime, date, timedelta
from dateutil.rrule import rruleset, rrulestr
from pytz import timezone
import msgflo

tz = timezone('Europe/Berlin')
now = datetime.now(tz)
recurFrom = datetime.now().replace(hour=0, minute=0, second=0, microsecond=0)
recurTo = recurFrom + timedelta(days=7)

def clean_event(event):
    if not event:
        return event
    return {
        'start': event['start'].isoformat(),
        'end': event['end'].isoformat(),
        'summary': event['summary'],
        'location': event['location'],
    }

def get_events(url):
    req = urllib.request.Request(url)
    res = urllib.request.urlopen(req)
    data = res.read()
    cal = icalendar.Calendar.from_ical(data)
    events = []
    for event in cal.walk('vevent'):
        start = event.get('dtstart')
        if not start:
            continue
        start = start.dt
        if (type(start) == date):
            start = tz.localize(datetime.combine(start, datetime.min.time()))
        end = event.get('dtend')
        if not end:
            continue
        end = end.dt
        if (type(end) == date):
            end = tz.localize(datetime.combine(end, datetime.max.time()))

        repeat = event.get('rrule')
        if repeat:
            rrule = rrulestr(repeat.to_ical().decode(), ignoretz=True, dtstart=start.replace(tzinfo=None))
            ruleset = rruleset()
            ruleset.rrule(rrule)
            nextRepeat = ruleset.between(recurFrom, recurTo)
            if (len(nextRepeat) == 0):
                continue
            start = nextRepeat[0].replace(tzinfo=tz)
            end = datetime(nextRepeat[0].year, nextRepeat[0].month, nextRepeat[0].day,
                                                             end.hour, end.minute, end.second, tzinfo=tz)

        if (end < now):
            continue

        events.append({
            'start': start,
            'end': end,
            'summary': event.get('summary'),
            'location': event.get('location'),
        })

    events.sort(key=lambda x: x['start'])
    currentEvent = None
    nextEvent = None
    for event in events:
        if (event['start'] < now and event['end'] > now):
            currentEvent = event
        if (event['start'] > now):
            nextEvent = event
            break
    return {
            'current': clean_event(currentEvent),
            'next': clean_event(nextEvent),
    }

class EventCalendar(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/EventCalendar',
      'label': 'Read c-base event calendar',
      'icon': 'calendar',
      'inports': [
        { 'id': 'in', 'type': 'bang' },
      ],
      'outports': [
        { 'id': 'current', 'type': 'object' },
        { 'id': 'next', 'type': 'object' },
      ],
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    events = get_events('https://www.c-base.org/calendar/exported/c-base-events.ics')
    self.send('current', events['current'])
    self.send('next', events['next'])
    self.ack(msg)

if __name__ == '__main__':
  msgflo.main(EventCalendar)
