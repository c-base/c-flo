msgflo = require 'msgflo-nodejs'
MetarFetcher = require('metar-taf').MetarFetcher
metarFetcher = new MetarFetcher
metarParser = require('metar')

getWeather = (station, callback) ->
  metarFetcher.getData(station)
  .then (data) ->
    clean = data.split("\n")[1]
    parsed = metarParser clean
    parsed.metar = clean
    callback null, parsed
  , (err) ->
    callback err
getHumidity = (weather) ->
  Tc = weather.temperature
  Tdc = weather.dewpoint
  Es=6.11*10.0**(7.5*Tc/(237.7+Tc))
  E=6.11*10.0**(7.5*Tdc/(237.7+Tdc))
  return (E/Es)*100

Participant = (client, role) ->
  station = null
  lastFetch = null
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
      if weather.time is lastFetch
        # Send only when there is new METAR
        callback 'skipped', null, weather
        return
      participant.send 'pressure', weather.altimeterInHpa
      participant.send 'humidity', getHumidity weather
      participant.send 'metar', weather.metar
      lastFetch = weather.time
      callback 'temperature', null, weather.temperature

  participant = new msgflo.participant.Participant client, definition, process, role
  return participant

module.exports = Participant
