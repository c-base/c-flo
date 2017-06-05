module.exports = ->
  grunt = @
  @initConfig
    pkg: @file.readJSON 'package.json'

    yamllint:
      participants: ['components/*.yml']
      specs: ['spec/*.yaml']
    createMarkup:
      participants: ['components/*.yml']
    pylint:
      options:
        errorsOnly: true
        rcfile: '.pylintrc'
      src: ['components/*.py']

    # BDD tests on Node.js
    mochaTest:
      nodejs:
        src: ['spec/*.js']
        options:
          reporter: 'spec'

  @loadNpmTasks 'grunt-yamllint'
  @loadNpmTasks 'grunt-pylint'
  @loadNpmTasks 'grunt-mocha-test'

  @task.registerMultiTask 'createMarkup', ->
    mqttArtifacts = []
    artifact = ""
    @files.forEach (file) ->
      artifact += "^ artifact ^ description ^ inports topic ^ outports topic^ \n"
      file.src.forEach (src) ->
        readYaml = grunt.file.readYAML src
        readYaml.component = readYaml.component.replace 'c-flo/', ''
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

  @registerTask 'test', [
    'yamllint'
    'pylint'
    'mochaTest'
    'createMarkup'
  ]
  @registerTask 'default', ['test']
