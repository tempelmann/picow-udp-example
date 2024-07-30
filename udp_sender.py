#!/usr/bin/python
#
# Part of https://github.com/tempelmann/picow-udp-example

# Set this to the Pico's IP address which it will print once it's connected to WiFi:
IP = "192.168.4.255" # use 255 at the last field for broadcast to work to the entire network

PORT=6002

# change this to test how much you can transfer (single packet limit is 1472):
PAYLOAD_SIZE=1460

import socket
import time
import random

random_list = []
for n in range(PAYLOAD_SIZE):
    random_list.append(random.randint(0, 255))
write_buf = bytearray(random_list)

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET,socket.SO_BROADCAST,1)

try:
    count = 1;
    while True:
       #msg = "Send to Pico {} times\n\0".format(count).encode('utf-8')
       msg = write_buf
       count=count+1
       sock.sendto(msg,(IP,PORT))
       time.sleep(1)
except KeyboardInterrupt:
    pass
