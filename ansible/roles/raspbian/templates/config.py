from uuid import getnode

important_urls = [
    "http://c-beam.cbrp3.c-base.org/he1display",
]

urls = [
  "http://www.c-base.org",
  "http://logbuch.c-base.org/",
  "http://c-beam.cbrp3.c-base.org/events",
  "https://c-beam.cbrp3.c-base.org/c-base-map",
  "http://cbag3.c-base.org/artefact",
  "https://c-beam.cbrp3.c-base.org/missions",
  "https://c-beam.cbrp3.c-base.org/weather",
  "http://c-beam.cbrp3.c-base.org/bvg",
  "http://c-beam.cbrp3.c-base.org/nerdctrl",
#  "https://c-beam.cbrp3.c-base.org/rickshaw/examples/fixed.html",
  "https://c-beam.cbrp3.c-base.org/sensors",
  "https://c-beam.cbrp3.c-base.org/ceitloch",
#  "http://visibletweets.com/#query=@cbase&animation=2",
  "https://c-beam.cbrp3.c-base.org/reddit",
  "http://vimeo.com/cbase/videos",
  "https://wiki.c-base.org/dokuwiki/",
  "https://github.com/c-base/meta/issues",
  "http://app.flowhub.io#runtime/endpoint?protocol%3Dwebsocket%26address%3Dws%3A%2F%2Fc-flo.cbrp3.c-base.org%3A3569%26id%3Da9dca883-c07f-4cd7-b369-180fa9b52b68",
]

mqtt_client_id = "{{ ansible_nodename }}"
mqtt_client_name = "{{ ansible_nodename }}"
mqtt_client_password = "ejwfoiwejfwofijf38fu98f1hfnwevlkwenvlwevjn"

mqtt_server = "c-beam.cbrp3.c-base.org"
mqtt_server_tls = False
#mqtt_server_cert = "/etc/ssl/certs/c-beam-mqtt.crt"
#mqtt_server_cert = "/etc/ssl/certs/c-beam-ext.c-base.org.pem"
mqtt_server_cert = "/etc/ssl/certs/root.crt"

discovery_message = {
  "command": "participant",
  "protocol": "discovery",
  "payload": {
    "component": "c-base/mqttwebview",
    "label": "Show URL on a public screen.",
    "inports": [
        {
            "queue": "%s/open" % mqtt_client_name,
            "type": "string",
            "description": "URL to be opened",
            "id": "open",
        }
    ],
    "outports": [
        {
            "queue": "%s/opened" % mqtt_client_name,
            "type": "object",
            "description": "The URL that has been opened and is showing.",
            "id": "opened",
        }
    ],
    "role": "%s" % mqtt_client_name,
    "id": mqtt_client_id,
    "icon": "television"
  }
}