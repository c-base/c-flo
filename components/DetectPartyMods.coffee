noflo = require 'noflo'

exports.getComponent = ->
  c = new noflo.Component
  c.inPorts.add 'state',
    datatype: 'object'
  c.outPorts.add 'lights',
    datatype: 'object'

  noflo.helpers.WirePattern c,
    in: 'state'
    out: 'lights'
  , (data, groups, out) ->
    data.mods = [] unless data.mods
    partyMods = data.mods.filter (m) ->
      m.type is 'Multi-hack' and m.rarity is 'Very Rare'
    if partyMods.length > 1
      # Hack Hack Hack!!!
      console.log "GREEN"
      out.send
        red: 0
        green: 1
      return
    if partyMods.length is 1
      # Would be nice to go yellow here
      console.log "YELLOW"
      out.send
        red: 1
        green: 1
      return
    console.log "RED"
    out.send
      red: 1
      green: 0
