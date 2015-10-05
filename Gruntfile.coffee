msgflo_nodejs = require 'msgflo-nodejs'
msgflo = require 'msgflo'
path = require 'path'

module.exports = ->
  grunt = @
  @initConfig
    pkg: @file.readJSON 'package.json'

    yamllint:
      participants: ['participants/*.yml']
    register:
      participants: ['participants/*.yml']

  @loadNpmTasks 'grunt-yamllint'
  @task.registerMultiTask 'register', ->
    done = @async()
    options = @options
      broker: 'mqtt://localhost'
    grunt.verbose.writeln "Connecting to MsgFlo broker #{options.broker}"
    messaging = msgflo_nodejs.transport.getClient options.broker
    connected = false
    setTimeout ->
      return if connected
      done new Error "Failed to connect to #{options.broker}"
    , 5000
    messaging.connect (err) =>
      return done err if err
      connected = true
      defs = []
      @files.forEach (file) ->
        file.src.forEach (src) ->
          def = grunt.file.readYAML src
          def.id = path.basename src, path.extname src unless def.id
          def.role = path.basename src, path.extname src unless def.role
          defs.push msgflo.foreignParticipant.mapPorts def
      todo = defs.length
      for def in defs
        grunt.log.writeln "Registering #{def.role} (#{def.component})"
        msgflo.foreignParticipant.register messaging, def, (err) ->
          return done err if err
          todo--
          return done() if todo < 1

  @registerTask 'test', ['yamllint']
  @registerTask 'default', ['test']
