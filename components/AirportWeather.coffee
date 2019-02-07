msgflo = require 'msgflo-nodejs'
adds = require("adds")

getWeather = (station, callback) ->
  adds('metars',
    stationString: station
    hoursBeforeNow: 1
  )
  .then (data) ->
    callback null, data[0]
  , (err) ->
    callback err
getHumidity = (weather) ->
  Tc = weather.temp_c
  Tdc = weather.dewpoint_c
  Es=6.11*10.0**(7.5*Tc/(237.7+Tc))
  E=6.11*10.0**(7.5*Tdc/(237.7+Tdc))
  return (E/Es)*100

Participant = (client, role) ->
  station = null
  lastMetar = null
  definition =
    id: role
    component: 'c-flo/AirportWeather'
    icon: 'plane'
    label: 'Fetch weather data for an airport'
    inports: [
      id: 'icao'
      type: 'string'
      hidden: false
    ,
      id: 'fetch'
      type: 'bang'
      hidden: false
    ,
      id: 'temperature'
      type: 'float'
      hidden: true
    ,
      id: 'pressure'
      type: 'float'
      hidden: true
    ]
    outports: [
      id: 'temperature'
      type: 'float'
      hidden: false
    ,
      id: 'humidity'
      type: 'float'
      hidden: false
    ,
      id: 'pressure'
      type: 'float'
      hidden: false
    ,
      id: 'metar'
      type: 'string'
      hidden: false
    ,
      id: 'error'
      type: 'object'
      hidden: false
    ,
      id: 'skipped'
      type: 'object'
      hidden: true
    ]
  process = (inport, indata, callback) ->
    if inport in ['temperature', 'humidity', 'pressure', 'metar']
      # Forward to outport
      return callback inport, null, indata
    unless inport in ['icao', 'fetch']
      return callback 'error', new Error "Unknown port name"
    if inport is 'icao'
      station = indata
    if inport is 'fetch' and station is null
      return callback 'error', new Error "No weather station provided"
    getWeather station, (err, weather) ->
      return callback 'error', err if err
      if weather.raw_text is lastMetar
        # Send only when there is new METAR
        callback 'skipped', null, weather
        return
      if weather.sea_level_pressure_mb < 500
        # Faulty reading, skip
        callback 'skipped', null, weather
        return
      participant.send 'pressure', weather.sea_level_pressure_mb
      participant.send 'humidity', getHumidity weather
      participant.send 'metar', weather.raw_text
      lastMetar = weather.raw_text
      callback 'temperature', null, weather.temp_c

  participant = new msgflo.participant.Participant client, definition, process, role
  return participant

module.exports = Participant
