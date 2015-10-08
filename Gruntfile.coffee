msgflo_nodejs = require 'msgflo-nodejs'
msgflo = require 'msgflo'
path = require 'path'

module.exports = ->
  grunt = @
  @initConfig
    pkg: @file.readJSON 'package.json'

    # Updating the package manifest files
    noflo_manifest:
      update:
        files:
          'package.json': ['graphs/*', 'components/*']

    yamllint:
      participants: ['participants/*.yml']
    updateforeign:
      participants: ['participants/*.yml']
    register:
      participants: ['participants/*.yml']
    createMarkup:
      participants: ['participants/*.yml']

  @loadNpmTasks 'grunt-yamllint'
  @loadNpmTasks 'grunt-noflo-manifest'
  @task.registerMultiTask 'updateforeign', ->
    conf = grunt.file.readJSON 'package.json'
    foreigns = []
    @files.forEach (file) ->
      file.src.forEach (src) ->
        foreigns.push path.basename src, path.extname src
    conf.msgflo = {} unless conf.msgflo
    conf.msgflo.components = {} unless conf.msgflo.components
    for k, v of conf.msgflo.components
      delete conf.msgflo.components[k] if v is '#FOREIGN'
    for f in foreigns
      conf.msgflo.components["#{conf.name}/#{f}"] = '#FOREIGN'
    grunt.file.write 'package.json', JSON.stringify(conf, null, 2), 'utf-8'

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
  
  @task.registerMultiTask 'createMarkup', ->
    mqttArtifacts = []
    artifact = ""
    @files.forEach (file) ->
      artifact += "^ artifact ^ description ^ inports topic ^ outports topic^ \n"
      file.src.forEach (src) ->
        readYaml = grunt.file.readYAML src
        readYaml.component = readYaml.component.replace 'c-base/', ''
        if readYaml.source?
          artifact += "|[[#{readYaml.source}|#{readYaml.component}]] "
        else
          artifact += "|#{readYaml.component} "
        artifact +="|#{readYaml.label}"
        if Object.keys(readYaml.inports).length == 0
          artifact += "| n/a "
        else
          artifact += "|"
          queues = []
          for key, val of readYaml.inports
            for key1, val1 of val
              if key1 == "queue"
                queues.push "**#{key}**: #{val1}"

          artifact += queues.join(' \\\\ ')

        if Object.keys(readYaml.outports).length == 0
          artifact += "| n/a "
        else
          artifact += "|"
          queues = []
          for key, val of readYaml.outports
            for key1, val1 of val
              valParts = val1.split '/'
              val1 = "#{valParts[0]}/#{valParts[1]}/+" if valParts[0] is 'ingress'
              if key1 == "queue"
                queues.push "**#{key}**: #{val1}"

          artifact += queues.join(' \\\\ ')
        artifact +="|\n"
      grunt.log.writeln "#{artifact}"

  @registerTask 'test', ['noflo_manifest', 'updateforeign', 'yamllint']
  @registerTask 'default', ['test']
