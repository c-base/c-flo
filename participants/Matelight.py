#!/usr/bin/env python

#cflo stuff
import sys, os, json, logging
import gevent
import msgflo
from itertools import cycle

#matelight stuff
import socket
import colorsys # for HSV-to-RGB-conversion
from PIL import Image, GifImagePlugin, ImageSequence, ImageOps
import time
import sys
import urllib

log = logging.getLogger(__name__)

HOSTNAME = "matelight.cbrp3.c-base.org"
UDP_PORT = 1337
TCP_PORT = 1337

ROWS = 16
COLS = 40

BRIGHTNESS = 1.0
GAMMA = 1.0

loop_last_frame = False

class Matelight(msgflo.Participant):
    def __init__(self, role):
        d = {
          'component': 'c-flo/Matelight',
          'label': 'Interface to Matelight',
          'icon': 'television',
          'inports': [
            { 'id': 'gif_url', 'type': 'string', },
            { 'id': 'text_string', 'type': 'string', },
          ],
          'outports': [
          ],
        }
        self.busy = False
        msgflo.Participant.__init__(self, d, role)

    def send_array(self, data, hostname):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(data, (hostname, UDP_PORT))

    def prepare_message(self, data, unpack=False, gamma=GAMMA):
        """Prepares the pixel data for transmission over UDP
        """
        # 4 bytes for future use as a crc32 checksum in network byte order.
        checksum = bytearray([0,0,0,0])
        data_as_bytes = bytearray()
        if unpack:
            for r, g, b, a in data:
                r = int(((r/255.0) ** gamma) * 255 * BRIGHTNESS)
                g = int(((g/255.0) ** gamma) * 255 * BRIGHTNESS)
                b = int(((b/255.0) ** gamma) * 255 * BRIGHTNESS)
                data_as_bytes += bytearray([r,g,b])
        else:
            data_as_bytes = bytearray(data)
            
        while len(data_as_bytes) < 1920:
            data_as_bytes += bytearray([0,0,0])
        
        message = data_as_bytes + checksum
        return message

    def show_gif(self, filename, hostname=HOSTNAME, gamma=GAMMA, centering=0.5):
        '''
        Plays gif file once then deletes the file and clears the busy flag
        '''
        img = Image.open(filename)
        palette = img.getpalette()
        last_frame = Image.new("RGBA", img.size)
        frames = []
        message = None
        
        for frame in ImageSequence.Iterator(img):
            #This works around a known bug in Pillow
            #See also: http://stackoverflow.com/questions/4904940/python-converting-gif-frames-to-png
            frame.putpalette(palette)
            c = frame.convert("RGBA")
            sleep_time = img.info['duration'] / 1000.0
            
            # print img.info['background'], img.info['transparency']
            try:
                if img.info['background'] != img.info['transparency']:
                    last_frame.paste(c, c)
                else:
                    last_frame.paste(c, c)
                    # last_frame = c
            except KeyError:
                last_frame = c 

            im = last_frame.copy()
            tw, th = im.size
            if (tw, th) != (40, 16):
                im = ImageOps.fit(im, (40, 16), Image.NEAREST, centering=(0.5, centering))
            else:
                pass

            data=list(im.getdata())
            message = self.prepare_message(data, unpack=True, gamma=gamma)
            self.send_array(message, hostname)     
            gevent.sleep(sleep_time)
        os.unlink = filename
        self.busy = False
    
    def sendText(self, msg):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOSTNAME, TCP_PORT))
        s.send(msg)
        
    def process(self, inport, msg):
        if inport is 'gif_url':
            log.info("Matelight gets gif")
            if not self.busy:
                self.busy = True
                try:
                    filename = urllib.urlretrieve(msg.data)[0]
                    gevent.Greenlet.spawn(self.show_gif, filename)
                finally:
                    self.ack(msg)
            else:
                log.info("Matelight is busy")
        elif inport is 'text_string':
            log.info("Matelight gets string: %s", msg)
            sendText(msg)
            


if __name__ == '__main__':
    msgflo.main(Matelight)
