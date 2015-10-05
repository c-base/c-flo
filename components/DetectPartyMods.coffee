noflo = require 'noflo'

exports.getComponent = ->
  c = new noflo.Component
  c.inPorts.add 'state',
    datatype: 'object'
  c.inPorts.add 'lights',
    datatype: 'object'

  noflo.helpers.WirePattern c,
    in: 'state'
    out: 'lights'
  , (data, groups, out) ->
    data.mods = [] unless data.mods
    partyMods = data.mods.filter (m) ->
      m.type is 'Multi-hack' and mod.rarity is 'Very Rare'
    if partyMods.length > 1
      # Hack Hack Hack!!!
      out.send
        red: 0
        green: 1
      return
    if partyMods.length is 1
      # Would be nice to go yellow here
      out.send
        red: 1
        green: 1
      return
    out.send
      red: 1
      green: 0
