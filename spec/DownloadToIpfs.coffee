noflo = require 'noflo'
chai = require 'chai'
path = require 'path'
baseDir = path.resolve __dirname, '../'

describe 'DownloadToIpfs component', ->
  c = null
  url = null
  hash = null
  error = null
  before (done) ->
    @timeout 20*1000
    loader = new noflo.ComponentLoader baseDir
    loader.load 'c-flo/DownloadToIpfs', (err, instance) ->
      return done err if err
      c = instance
      done()
  beforeEach ->
    url = noflo.internalSocket.createSocket()
    hash = noflo.internalSocket.createSocket()
    error = noflo.internalSocket.createSocket()
    c.inPorts.url.attach url
    c.outPorts.hash.attach hash
    c.outPorts.error.attach error
  afterEach ->
    c.inPorts.url.detach url
    c.outPorts.hash.detach hash
    c.outPorts.error.detach error

  describe 'receiving a valid URL', ->
    it 'should produce IPFS hash', (done) ->
      @timeout 200 * 1000
      error.on 'data', done
      hash.on 'data', (data) ->
        chai.expect(data).to.equal 'Qmei9gT6df2oc7EGtifNAYL6MMqYT3WdHiCBycHFsM6zVU'
        done()
      url.send 'http://bergie.iki.fi/style/img/xhdpi/bergie_istanbul_small.jpg'
  describe 'receiving an invalid URL', ->
    it 'should produce an error', (done) ->
      @timeout 200 * 1000
      error.on 'data', (err) ->
        chai.expect(err).to.be.an 'error'
        done()
      hash.on 'data', (d) ->
        return done new Error "Received unexpected hash #{d}"
      url.send 'http://bergie.iki.fi/this-does-not-exist.jpg'
