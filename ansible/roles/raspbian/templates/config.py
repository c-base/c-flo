from uuid import getnode

important_urls = [{% for important_url in important_urls %}
    "{{ important_url }}",
{% endfor %}]

urls = [{% for url in urls %}
    "{{ url }}",
{% endfor %}]

mqtt_client_id = "{{ msgflo_role }}-{{ ansible_nodename }}"
mqtt_client_name = "{{ msgflo_role }}"
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