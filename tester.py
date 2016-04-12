#!/usr/bin/env python
# coding: utf-8

from __future__ import print_function
import time
from collections import defaultdict
from struct import *
from RF24 import *
from RF24Network import *

import sys
import datetime

global alarm_level
global sleep_cycles
global alarm_profile
global send_config
global config_queue


global radio
global network

# CE Pin, CSN Pin, SPI Speed
radio = RF24(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ)

network = RF24Network(radio)

channel = 100
interval = 500

millis = lambda: int(round(time.time() * 1000))
octlit = lambda n:int(n, 8)

# Address of our node in Octal format (01, 021, etc)
this_node = octlit("00")

radio.begin()
time.sleep(0.1)
radio.setPALevel(RF24_PA_MAX)
time.sleep(0.1)
network.begin(channel, this_node)
radio.printDetails()

def send_reply(node, messageno):
    global config_queue
    network.update()
    #print("send_reply to " + str(node) +" , messageno: " + str(messageno))
    payload = pack('<H', messageno)
    type = ord('O')
    #print('Sending ' + chr(type) + ' to ' + str(node) + '...')
    ok = network.write(RF24NetworkHeader(octlit(node), type), payload)
    if ok:
        print('o ok.')
    else:
        print('o failed.')

lastmessageno = 0

while 1:
    network.update()
    while network.available():
        network.update()
        rfheader = RF24NetworkHeader()

        network.peek(rfheader)

        if chr(rfheader.type) == 'M':
            header, payload = network.read(32)
            ''' remove these commented lines to get transmit work ok
            alarmSensortypes = []
            tempSensortypes = []
            humidities = []
            temps = []
            alarms = []

            alarmSensortypes = unpack('<BBB', bytes(payload)[0:3])
            tempSensortypes = unpack('<BBBB', bytes(payload)[3:7])
            humidities = list(unpack('<HHHH', bytes(payload)[7:15]))
            humidities = [float(i)/100 for i in humidities]
            temps = list(unpack('<hhhh', bytes(payload)[15:23]))
            temps = [float(i)/100 for i in temps]
            batterylevel = float(unpack('<H', bytes(payload)[23:25])[0])/1000
            alarmlevel = unpack('<B', bytes(payload)[25:26])[0]
            sleepcycles = unpack('<B', bytes(payload)[26:27])[0]
            alarms = unpack('<???', bytes(payload)[27:30])
            '''
            messageno = unpack('<H', bytes(payload)[30:32])[0]

            print('\n\n------------------------------------')
            print(datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d %H:%M:%S'))
            print('Measurements from:', oct(header.from_node))
            print('------------------------------------')
            print('Message No: ', messageno)

            send_reply(str(oct(header.from_node)), messageno)
        else:
            print('receive something else: ' + str(chr(rfheader.type)))
            header, payload = network.read(32)
        rfheader = None
    time.sleep(0.75)
