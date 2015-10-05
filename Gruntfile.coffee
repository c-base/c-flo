module.exports = ->
  @initConfig
    pkg: @file.readJSON 'package.json'

    yamllint:
      participants: ['participants/*.yml']

  @loadNpmTasks 'grunt-yamllint'
  @registerTask 'test', ['yamllint']
  @registerTask 'default', ['test']
