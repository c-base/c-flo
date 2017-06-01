#!/usr/bin/python
# coding=utf-8

import time
import struct
from time import gmtime, strftime
from colour import Color
import msgflo
import gevent

def generate_terminal_output(palette):
    print palette['time_value']
    print "base_color ", palette['base_color'].hex
    print "baseColorVariant1 ", palette['base_color_variant_1'].hex
    print "baseColorVariant2 ", palette['base_color_variant_2'].hex
    print "baseColorVariant3 ", palette['base_color_variant_3'].hex
    print "baseColorVariant4 ", palette['base_color_variant_4'].hex
    print "contrast_color ", palette['contrast_color'].hex
    print "###################################"

def generate_html_output(palette):
    time_value = palette['time_value']
    base_color = palette['base_color']
    base_color_variant_1 = palette['base_color_variant_1']
    base_color_variant_2 = palette['base_color_variant_2']
    base_color_variant_3 = palette['base_color_variant_3']
    base_color_variant_4 = palette['base_color_variant_4']
    contrast_color = palette['contrast_color']

    htmlpreface = """<html><head><title>visuelle Ausgabeeinheit des zentralen Farbgebers</title><meta http-equiv="refresh" content="1" />
    <style type="text/css">
    """
    htmlcontent = """</style></head><body><h1>visuelle Ausgabeeinheit des zentralen Farbgebers</h1>
    <div>BaseColor """ + base_color.hex + """</div></ br>
    <div class="base_color_variant_1">baseColorVariant1 """ + base_color_variant_1.hex + """</div>
    <div class="base_color_variant_2">baseColorVariant2 """ + base_color_variant_2.hex + """</div>
    <div class="base_color_variant_3">baseColorVariant3 """ + base_color_variant_3.hex + """</div>
    <div class="base_color_variant_4">baseColorVariant4 """ + base_color_variant_4.hex + """</div>
    <div class="Contrastcolor">Contrastcolor """ + contrast_color.hex + """</div>"""
    zeitzeile = "<h3>Color-Seed " + str(time_value) + " " + strftime("%H:%M:%S", gmtime()) + "Uhr</h3>"
    htmlclosing = """</body></html>"""
    css1 = "body { font-size:20px; background-color:" + base_color.hex + "; color:" + contrast_color.hex + "; }"
    css2 = ".base_color_variant_1 { background-color:" + base_color_variant_1.hex + "; width:100%; height:40px; padding: 40px; font-size:20px; } \n\r"
    css3 = ".base_color_variant_2 { background-color:" + base_color_variant_2.hex + "; width:50%; height:40px; padding: 40px; font-size:20px; } \n\r"
    css4 = ".base_color_variant_3 { background-color:" + base_color_variant_3.hex + "; width:100%; height:40px; padding: 40px; font-size:20px; } \n\r"
    css5 = ".base_color_variant_4 { background-color:" + base_color_variant_4.hex + "; width:50%; height:40px; padding: 40px; font-size:20px; } \n\r"
    css6 = ".Contrastcolor { background-color:" + contrast_color.hex + "; width:10%; height:900px; position:absolute; right:300px; top:0px; color:" + base_color.hex + "; padding: 40px; font-size:20px; } \n"
    f = open('/home/coon/public_html/farbgeber.html', 'w')
    outputtxt = str(htmlpreface) + str(css1) + str(css2) + str(css3) + str(css4) + str(css5) + str(css6) + str(
        htmlcontent) + str(zeitzeile) + str(htmlclosing)
    f.write(outputtxt)
    f.close()

def generate_palette(base_saturation=1.0, base_luminance=0.4, hue_modifier=0.03, lum_modifier=0.07, sat_modifier=0.2): 
    time_value = int(strftime("%M", gmtime())) * 60 + int(strftime("%S", gmtime()))
    time_value = float(time_value)

    base_hue = time_value / 3600
    base_color = Color(hsl=(base_hue, base_saturation, base_luminance))        
    base_color_variant_1 = Color(hsl=(base_color.hue + hue_modifier, base_saturation - sat_modifier, base_luminance))
    base_color_variant_2 = Color(hsl=(base_color.hue - hue_modifier, base_saturation - sat_modifier, base_luminance))
    base_color_variant_3 = Color(hsl=(base_color.hue, base_saturation, base_luminance + lum_modifier))
    base_color_variant_4 = Color(hsl=(base_color.hue, base_saturation, base_luminance - lum_modifier))

    base_degree = base_hue * 360
    if base_degree < 180:
        contrast_hue = base_degree + 180
    else:
        contrast_hue = base_degree - 180
        
    contrast_hue /= 360
    contrast_color = Color(hsl=(contrast_hue, base_saturation - sat_modifier, (base_luminance + 0.2)))

    p = dict()
    p['time_value'] = time_value
    p['base_color'] = base_color        
    p['base_color_variant_1'] = base_color_variant_1
    p['base_color_variant_2'] = base_color_variant_2
    p['base_color_variant_3'] = base_color_variant_3
    p['base_color_variant_4'] = base_color_variant_4
    p['contrast_color'] = contrast_color

    return p

class Farbgeber(msgflo.Participant):
    def __init__(self, role):
        d = {
            'component': 'c-flo/farbgeber',
            'label': 'Produce pleasing color palettes',
            'icon': 'tint',
            'inports': [
                { 'id': 'in', 'type': 'bang' },
            ],
            'outports': [
                { 'id': 'palette', 'type': 'object' },
            ],
        }
        msgflo.Participant.__init__(self, d, role)

    def process(self, inport, msg):
        self.ack(msg)
        gevent.Greenlet.spawn(self.loop)

    def loop (self):
        while True:
            palette = generate_palette()
            self.send_palette(palette)
            gevent.sleep(1)

    def send_palette(self, palette):
        def packedColor(color):
            FLOAT_ERROR = 0.0000005

            def colorToInt(c):
                return int(c*255 + 0.5 - FLOAT_ERROR)

            return [colorToInt(color.get_red()), colorToInt(color.get_green()), colorToInt(color.get_blue())]

        data = dict()
        data['b'] = packedColor(palette['base_color'])
        data['v1'] = packedColor(palette['base_color_variant_1'])
        data['v2'] = packedColor(palette['base_color_variant_2'])
        data['v3'] = packedColor(palette['base_color_variant_3'])
        data['v4'] = packedColor(palette['base_color_variant_4'])
        data['c'] = packedColor(palette['contrast_color'])

        self.send('palette', data)

if __name__ == "__main__":
    print "Zentrale Farbgebeeinheit"
    msgflo.main(Farbgeber)
