# c-flo

[MsgFlo](https://github.com/msgflo/msgflo#readme) setup for the c-base space station

## Existing Participants

c-base has several artifacts communicating over the MQTT network. These are represented in MsgFlo-land as "foreign participants" loaded from definitions in the `participants/` folder.

To document any missing artifacts, please amend the definitions there. See [participant discovery](https://github.com/msgflo/msgflo#participant-discovery) for format documentation.

## Dynamic Participants

Dynamic MsgFlo participants can be defined in any of the [compatible languages](https://github.com/msgflo) and then connected to the graph.

For example, to start a simple NoFlo participant, run:

```
./node_modules/.bin/noflo-runtime-msgflo --name Log --graph core/Output --prefetch 1 --broker mqtt://localhost
```

## Examples

Output the time from c-beam time server:

    export MSGFLO_BROKER=mqtt://c-beam.cbrp3.c-base.org
    ./node_modules/.bin/msgflo-setup graphs/timelogger.fbp --participants --discover --forever --forward stderr,stdout

Make traffic lights in the downstairs hallway turn green when c-base portal has Ingress party mods:

    export MSGFLO_BROKER=mqtt://c-beam.cbrp3.c-base.org
    ./node_modules/.bin/msgflo-setup graphs/hacklight.fbp --participants --discover --forever --forward stderr,stdout
