# c-flo [![Build Status](https://travis-ci.org/c-base/c-flo.svg?branch=master)](https://travis-ci.org/c-base/c-flo) [![Greenkeeper badge](https://badges.greenkeeper.io/c-base/c-flo.svg)](https://greenkeeper.io/)

[MsgFlo](https://msgflo.org) setup for rewiring the [c-base space station](https://c-base.org/).

## Introduction

c-base is a crashed space station under Berlin that also happens to be one of the oldest hackerspaces. The station has lots of information systems and devices connected using the c-beam MQTT system. c-flo uses MsgFlo and [Flowhub](https://flowhub.io) to make those devices and the connections between them visible. It also allows new connections between devices to be made, either directly, or using simple Python components to convert data in-between.

The aim of c-flo is to make all systems at c-base visible in a single graph, and to foster creativity in how these systems and devices interact.

## Parts of the system

* [c-beam](https://wiki.c-base.org/dokuwiki/projects:c-beam) information transmission system over MQTT
* c-flo, the [MsgFlo coordinator](https://msgflo.org) setup for c-base
* Raspberry Pi 3 running c-flo
* [Flowhub](https://flowhub.io) IDE for viewing and reprogramming the c-flo network

## Access

The c-flo system is available in c-base crew network. You can open the graph with the following URL:

<http://app.flowhub.io#runtime/endpoint?protocol%3Dwebsocket%26address%3Dws%3A%2F%2Fc-flo.cbrp3.c-base.org%3A3569%26id%3Da9dca883-c07f-4cd7-b369-180fa9b52b68>

This will download the Flowhub app to your browser and connect directly with the c-flo coordinator.

## Connecting Systems

Each system or device at c-base can be programmed to appear as a node in the c-flo graph by making it a MsgFlo participant. The design principles and how to make systems visible to c-flo are documented below.

### Design Principles

All devices and systems at c-base should interface with the c-beam MQTT network. Optimally this means receiving their input from a MQTT topic, and sending their status or output to another MQTT topic.

To maximize interoperability, systems should use their own MQTT topics and not be "hardcoded" to speak with a particular other system. `artifact-name/functionality` is a good naming scheme. For example `display/open_url` for a topic where a screen receives URLs to show, and `display/opened` for a topic where it sends what it is currently displaying.

The MQTT topic names should be made so that there can be multiple instances of same system running. So you might have `display1/open` and `display_downstairs/open`.

Connections between systems should be made visually using c-flo.

When needed, Python components can be written as "glue" to convert data from one system to the format wanted by another.

Modifications made to the c-flo network should be committed to this repository.

### Existing Participants

When the c-flo effort started, c-base already had several artifacts communicating over the MQTT network. These are represented in MsgFlo-land as ["foreign participants"](https://msgflo.org/docs/foreign/index.html) loaded from YAML definitions in the `components/` folder.

To document any missing artifacts, please amend the definitions there. See [participant discovery](https://msgflo.org/docs/communications/index.html) for format documentation.

For example, here is how the Echelon network monitoring system was made to appear in c-flo:

```yaml
component: c-flo/echelon
label: station network traffic monitoring
icon: wifi
inports: {}
outports:
  traffic:
    queue: system/echelon/traffic
    type: object
```

The foreign participant mechanism is fine for making existing systems appear in c-flo. However, any new artifacts should be made to announce themselves.

### Self-announcing Participants

If you're building a new system that interfaces over c-beam, it is quite easy to make it self-announce itself on c-flo. This is done by periodically sending a [MsgFlo discovery message](https://msgflo.org/docs/communications/index.html) on the `fbp` MQTT topic.

The discovery messages are formatted as JSON and contain the following information:

* Component name (typically the GitHub project name, like `c-base/ingress-table`). There can be multiple devices running the same component
* Role, the name of the node in the c-flo graph. This is used to distinguish different instances of same system, so `siri` and `he1` can run same software component but are different devices
* Inports, listing the topics the system listens to
* Outports, listing the topics the system writes to

For example, here is the discovery message sent by [farbgeber](https://github.com/c-base/farbgeber):

```json
{
  "command": "participant",
    "protocol": "discovery",
    "payload": {
      "component": "c-base/farbgeber",
      "inports": [
      {
        "queue": "farbgeber.IN",
        "type": "bang",
        "id": "in"
      }
      ],
      "label": "Produce pleasing color palettes",
      "outports": [
      {
        "queue": "farbgeber.PALETTE",
        "type": "object",
        "id": "palette"
      }
      ],
      "role": "farbgeber",
      "id": "farbgeber92490",
      "icon": "tint"
    }
}
```

These discovery messages should be sent when the system starts up and connects to the c-base bot network. In addition it should be re-sent roughly once per minute.

There are MsgFlo libraries available for [various programming languages](https://github.com/msgflo) to handle the discovery flow automatically.

Some examples of self-announcing participants:

* [mqttwebview URL displayed](https://github.com/c-base/mqttwebview)
* [Farbgeber client for ESP8266 microcontrollers](https://github.com/c-base/farbgeber/tree/master/esp8266/mqtt_client)

### Dynamic Participants

Dynamic participants are started by c-flo itself on-demand. These are typically [msgflo-python](https://github.com/msgflo/msgflo-python) components used as glue to convert data between other systems or to add dynamic logic to the network.

Some examples of dynamic participants:

* [NetworkBars](https://github.com/c-base/c-flo/blob/master/components/NetworkBars.py) converts current c-base network traffic to a DMX light visualization
* [DetectABBA](https://github.com/c-base/c-flo/blob/master/components/DetectABBA.py) tells whether a currently playing song is by ABBA
* [VisualPaging](https://github.com/c-base/c-flo/blob/master/components/VisualPaging.py) shows current spoken announcements as web pages on connected displays

## Testing Participants

Dynamic participants included in this repository can be included in our test automation setup. Tests are written in [fbp-spec](https://github.com/flowbased/fbp-spec#writing-tests) format and stored as `.yaml` files in the `spec/` folder. For example, here is how we can test the ABBA detector:

```yaml
name: 'Detecting ABBA'
topic: c-flo/DetectABBA
fixture:
  type: 'fbp'
  data: |
    INPORT=detect.SONG:IN
    OUTPORT=detect.OUT:OUT
    detect(c-flo/DetectABBA)
cases:
-
  name: 'currently playing AC/DC'
  assertion: 'should return false'
  inputs:
    in:
      artist: 'AC/DC'
  expect:
    out:
      -
        equals: false
-
  name: 'currently playing ABBA'
  assertion: 'should return true'
  inputs:
    in:
      artist: 'ABBA'
  expect:
    out:
      -
        equals: true
```

To run tests locally, `npm install` this project, start up a local mosquitto process, and then:

```shell
$ MSGFLO_BROKER=mqtt://localhost npm test
```

## create Markup for WIKI

Command  `grunt createMarkup` will generate a table from the participants folder that can be copied and inserted into the mqtt wiki page (https://wiki.c-base.org/dokuwiki/projects:mqtt).

* TODO: automate the process, so the wiki is always up to date
* TODO: add support for FontAwesome to wiki.
