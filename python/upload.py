#!/usr/bin/env python3
import argparse
import sys
import os.path
import serial
import threading
import time


port = '/dev/ttyUSB0'
port1 = '/dev/ttyUSB1'

if not os.path.exists(port):
    port = port1

infile = "prog.txt"

args = argparse.ArgumentParser(description='Minimal assembler for DOG-1.')
args.add_argument('-i','--input-file', action="store", dest='input')
args.add_argument('-p','--port', action="store", dest='port')
args_dict = vars(args.parse_args())
if args_dict["input"]:
    infile = args_dict["input"]
if args_dict["port"]:
    port = args_dict["port"]

# print(port)


ser = serial.Serial(port, 57600) # 9600

# TODO replace with handshake
print("Sleeping while Arduino reboots...")
time.sleep(8)

data = ""
startMarker = 60 # <
endMarker = 62 # >

data = data + "<"

with open(infile, "r") as ins:
    for line in ins:
        data = data + line[:2].rstrip()

data = data + ">"

ser.write(data.encode('utf-8'))

ser.close()
sys.stdout.write(data)