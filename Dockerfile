FROM node:6

# Export the Websocket port for Flowhub connection
EXPOSE 3569

# Reduce npm install verbosity, overflows Travis CI log view
ENV NPM_CONFIG_LOGLEVEL warn

RUN mkdir -p /var/app
WORKDIR /var/app

COPY . /var/app

# Install msgflo-python
RUN apt-get update && apt-get install -y \
  python \
  python-dev \
  python-pip
RUN pip install -r requirements.txt

# Install MsgFlo and dependencies
RUN npm install

# Map the volumes
VOLUME /var/app/graphs /var/app/components /var/app/spec

CMD npm start
