var msgflo = require('msgflo-nodejs');

module.exports = function (client, role) {
  var definition = {
    component: 'c-base/togglelights',
    icon: 'toggle-on',
    label: 'Switch traffic light states on every packet',
    inports: [
      {
        id: 'in',
        type: 'bang'
      }
    ],
    outports: [
      {
        id: 'out',
        type: 'object'
      }
    ]
  };

  var state = {
    red: 1,
    yellow: 0,
    green: 0
  };

  var process = function (inport, indata, callback) {
    if (state.red) {
      state.red = 0;
    } else {
      state.red = 1;
    }
    if (state.green) {
      state.green = 0;
    } else {
      state.green = 1;
    }
    return callback('out', null, {
      red: state.red,
      yellow: state.yellow,
      green: state.green
    });
  };

  return new msgflo.participant.Participant(client, definition, process, role);
};
