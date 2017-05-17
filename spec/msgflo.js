// Integrate fbp-spec (.yaml files) with Mocha test runner
const fbpspec = require('fbp-spec');
const runtime = {
  label: "c-flo",
  description: "",
  type: "msgflo",
  protocol: "websocket",
  address: "ws://localhost:3335",
  secret: "not-secret-3335",
  id: "d4a645ae-beeb-4c04-9bb3-4dfb5e063ebe",
  command: "./node_modules/.bin/msgflo --port=3335 --host=localhost --broker=" + process.env.MSGFLO_BROKER,
};

const options = {
  starttimeout: 60 * 1000,
  fixturetimeout: 60 * 1000
};
describe('c-flo participants', function() {
  fbpspec.mocha.run(runtime, './spec', options);
});
