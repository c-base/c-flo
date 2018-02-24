import requests
import msgflo

endpoint = 'https://api.luftdaten.info/v1/push-sensor-data/'

class SendToLuftDaten(msgflo.Participant):
  def __init__(self, role):
    d = {
      'component': 'c-flo/SendToLuftDaten',
      'label': 'Send environmental data to luftdaten.info',
      'icon': 'cloud-upload',
      'inports': [
        { 'id': 'pm10', 'type': 'number' },
        { 'id': 'pm25', 'type': 'number' },
        { 'id': 'temperature', 'type': 'number' },
        { 'id': 'humidity', 'type': 'number' },
        { 'id': 'sensor', 'type': 'string' },
      ],
      'outports': [
        { 'id': 'sent', 'type': 'bang' },
        { 'id': 'skipped', 'type': 'bang' },
      ],
    }
    self.values = {
      'pm10': None,
      'pm25': None,
      'temperature': None,
      'humidity': None
    }
    self.sensorId = 'msgflo-000042'
    msgflo.Participant.__init__(self, d, role)

  def process(self, inport, msg):
    if inport == 'sensor':
      self.sensorId = msg.data
      self.ack(msg)
      return
    self.values[inport] = msg.data
    if inport == 'pm10' or inport == 'pm25':
      print(self.values)
      if self.values['pm10'] != None and self.values['pm25'] != None:
        self.sendToLuftDaten(1, {
          'P1': self.values['pm10'],
          'P2': self.values['pm25'],
        })
        self.values['pm10'] = None
        self.values['pm25'] = None
        self.send('sent', True)
      else:
        self.send('skipped', True)
    if inport == 'temperature' or inport == 'humidity':
      print(self.values)
      if self.values['temperature'] != None and self.values['humidity'] != None:
        self.sendToLuftDaten(11, {
          'temperature': self.values['temperature'],
          'humidity': self.values['humidity'],
        })
        self.values['temperature'] = None
        self.values['humidity'] = None
        self.send('sent', True)
      else:
        self.send('skipped', True)
    self.ack(msg)

  def sendToLuftDaten(pin, values):
    headers = {
      'X-Pin': str(pin),
      'X-Sensor': self.sensorId
    }
    json = {
      'software_version': 'microflo-luftdaten 1.0.0',
      "sensordatavalues": [{'value_type': key, 'value': val} for key, val in values.items()],
    }
    requests.post(endpoint, headers=headers, json=json)

if __name__ == '__main__':
  msgflo.main(SendToLuftDaten)
