# c-flo [![Build Status](https://travis-ci.org/c-base/c-flo.svg?branch=master)](https://travis-ci.org/c-base/c-flo) [![Greenkeeper badge](https://badges.greenkeeper.io/c-base/c-flo.svg)](https://greenkeeper.io/)

[MsgFlo](https://msgflo.org) setup for rewiring the [c-base space station](https://c-base.org/).

## Introduction

c-base is a crashed space station under Berlin that also happens to be one of the oldest hackerspaces. The station has lots of information systems and devices connected using the c-beam MQTT system. c-flo uses MsgFlo and [Flowhub](https://flowhub.io) to make those devices and the connections between them visible. It also allows new connections between devices to be made, either directly, or using simple Python components to convert data in-between.

## Parts of the system

* [c-beam](https://wiki.c-base.org/dokuwiki/projects:c-beam) information transmission system over MQTT
* c-flo, the [MsgFlo coordinator](https://msgflo.org) setup for c-base
* Raspberry Pi 3 running c-flo
* [Flowhub](https://flowhub.io) IDE for viewing and reprogramming the c-flo network

## Access

The c-flo system is available in c-base member network. You can open the graph with the following URL:

<http://app.flowhub.io#runtime/endpoint?protocol%3Dwebsocket%26address%3Dws%3A%2F%2Fc-flo.cbrp3.c-base.org%3A3569%26id%3Da9dca883-c07f-4cd7-b369-180fa9b52b68>

This will download the Flowhub app to your browser and connect directly with the c-flo coordinator.

## Existing Participants

c-base has several artifacts communicating over the MQTT network. These are represented in MsgFlo-land as "foreign participants" loaded from definitions in the `participants/` folder.

To document any missing artifacts, please amend the definitions there. See [participant discovery](https://github.com/msgflo/msgflo#participant-discovery) for format documentation.

## Dynamic Participants

Dynamic MsgFlo participants can be defined in any of the [compatible languages](https://github.com/msgflo) and then connected to the graph.

For example, to start a simple NoFlo participant, run:

```
./node_modules/.bin/noflo-runtime-msgflo --name Log --graph core/Output --prefetch 1 --broker mqtt://localhost
```

# Installing & setup

    npm install

## Examples

Output the time from c-beam time server:

    export MSGFLO_BROKER=mqtt://c-beam.cbrp3.c-base.org
    ./node_modules/.bin/msgflo-setup graphs/timelogger.fbp --participants --discover --forever --forward stderr,stdout --shell=/bin/bash

Make traffic lights in the downstairs hallway turn green when c-base portal has Ingress party mods, and set up Siri downloader:

    export MSGFLO_BROKER=mqtt://c-beam.cbrp3.c-base.org
    ./node_modules/.bin/msgflo-setup graphs/c-base-noflo.fbp --participants --discover --forever --forward stderr,stdout --shell=/bin/bash

## create Markup for WIKI

Command  `grunt createMarkup` will generate a table from the participants folder that can be copied and inserted into the mqtt wiki page (https://wiki.c-base.org/dokuwiki/projects:mqtt).

* TODO: automate the process, so the wiki is always up to date
* TODO: add support for FontAwesome to wiki.
