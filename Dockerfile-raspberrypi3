FROM resin/raspberrypi3-node:8

# Export the Websocket port for Flowhub connection
EXPOSE 3569

# Reduce npm install verbosity, overflows Travis CI log view
ENV NPM_CONFIG_LOGLEVEL warn
ENV NODE_ENV production

# Install msgflo-python, freetype, jpeg and z-libs used for Pillow (Python Imaging Library)
RUN apt-get update && apt-get install -y \
  python3 \
  python3-dev \
  python3-pip \
  libjpeg-dev \
  zlib1g-dev \
  libfreetype6 \
  libfreetype6-dev

# Install python3 dependencies  
WORKDIR /
COPY ./requirements.pip /
RUN pip3 install -r /requirements.pip ; rm -f /requirements.pip

# Copy and install c-flo itself
RUN mkdir -p /var/c-flo
WORKDIR /var/c-flo
COPY . /var/c-flo

# Install MsgFlo and dependencies
RUN npm install --only=production

# Map the volumes
VOLUME /var/c-flo/graphs /var/c-flo/components /var/c-flo/spec

# Ensure that runtime is working
HEALTHCHECK --interval=5m --timeout=3s \
  CMD ./node_modules/.bin/fbp-protocol-healthcheck ws://127.0.0.1:3569

CMD npm start
