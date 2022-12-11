#!/usr/bin/python3

import serial
import threading
import time

# serial connection
ser = serial.Serial('/dev/ttyACM0')  # open serial port 
# time.sleep(4)
# numbytes= ser.in_waiting
# print(f"bytes available {numbytes}")
line = ser.readline()   # read a '\n' terminated line
print(line)
# numbytes= ser.in_waiting
# print(f"bytes available {numbytes}")
ser.close()