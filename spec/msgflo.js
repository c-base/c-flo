// Integrate fbp-spec (.yaml files) with Mocha test runner
const fbpspec = require('fbp-spec');
const runtime = {
  label: "c-flo",
  description: "",
  type: "msgflo",
  protocol: "websocket",
  address: "ws://localhost:3569",
  secret: "not-secret-3335",
  id: "762ac086-3012-4b20-a7ca-8db6eed21e88"
};

const options = {
  starttimeout: 60 * 1000,
  fixturetimeout: 60 * 1000
};
describe('c-flo participants', function() {
  fbpspec.mocha.run(runtime, './spec', options);
});
