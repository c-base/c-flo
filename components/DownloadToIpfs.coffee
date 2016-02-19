noflo = require 'noflo'
needle = require 'needle'
temp = require 'temp'
ipfsApi = require 'ipfs-api'

exports.getComponent = ->
  c = new noflo.Component
  c.description = 'Download something from an URL and add it to IPFS'
  c.icon = 'cloud-download'

  c.inPorts.add 'url',
    datatype: 'string'
  c.outPorts.add 'hash',
    datatype: 'string'
  c.outPorts.add 'error',
    datatype: 'object'

  noflo.helpers.WirePattern c,
    in: 'url'
    out: 'hash'
    async: true
    forwardGroups: true
  , (data, groups, out, callback) ->
    unless typeof data is 'string'
      return callback new Error 'Expected string, got something else'

    ipfs = ipfsApi() # '10.0.1.36'

    errored = false
    tempStream = temp.createWriteStream()
    req = needle.get data
    req.on 'error', (err) ->
      console.log "Request error for #{data}", err
      callback err
    req.once 'readable', ->
      if req.request.res.statusCode isnt 200
        console.log "Request error for #{data}", req.request.res.statusCode
        return callback new Error "#{data} responded with #{req.request.res.statusCode}"
      req.pipe tempStream
      tempStream.on 'finish', ->
        ipfs.add tempStream.path, (err, res) ->
          return callback err if err
          console.log "#{data} -> #{res[0].Hash}"
          out.send
            url: data
            ipfs: res[0].Hash
          do callback
