#!/usr/bin/env python
import urllib
from bs4 import BeautifulSoup
  
def available(availability):
  if availability == 'verfuegbar':
    return True
  else:
    return False

class Replicator(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/Replicator',
      'label': 'Gets the replicator status as json',
      'icon': 'tachometer',
      'inports': [
      	{ 'id': 'in', 'type': 'bang'}
      ],
      'outports': [
      	{ 'id': 'out', 'type': 'array'}
      ]
    }
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):

    url = 'http://replicator.cbrp3.c-base.org/status'
    response = urllib.urlopen(url)
    data = response.read()
    soup = BeautifulSoup(data, "html.parser")
    table = soup.find_all('table')[0]
    tr_list = table.find_all("tr")

    mate = tr_list[3].find_all("td")
    berliner1 = tr_list[4].find_all("td")
    berliner2 = tr_list[5].find_all("td")
    flora_mate = tr_list[6].find_all("td")
    premium_cola = tr_list[7].find_all("td")
    spetzi = tr_list[8].find_all("td")
    kraftmalz = tr_list[9].find_all("td")


    dict = {
      mate[0].text: available(mate[1].text.strip()),
      berliner1[0].text: available(berliner1[1].text.strip()),
      berliner2[0].text: available(berliner2[1].text.strip()),
      flora_mate[0].text: available(flora_mate[1].text.strip()),
      premium_cola[0].text: available(premium_cola[1].text.strip()),
      spetzi[0].text: available(spetzi[1].text.strip()),
      kraftmalz[0].text: available(kraftmalz[1].text.strip())
    }

    self.send('out', dict)
    self.ack(msg)