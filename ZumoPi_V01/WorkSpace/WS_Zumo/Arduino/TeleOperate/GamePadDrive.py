#!/usr/bin/python3
import threading
import serial
import time
import evdev
from evdev import InputDevice, categorize, ecodes

# serial connection
ser = serial.Serial('/dev/ttyACM0')  # open serial port
#creates object 'gamepad' to store the data
#you can call it whatever you like
gamepad = InputDevice('/dev/input/event0')

#prints out device info at start
print(gamepad)
global joyX , joyY

def TransmitThread(counter):
  while ser.isOpen:
    #print("send data")
    global joyX, joyY
    #print(f'joystick {joyX}\t{joyY}')
    #counter+=1
    #msg = 'test ' + str(counter) + '\r\n'
    msg = ''
    msg = str(joyX*2) + ',' +str(joyY*2) +'\r\n'
    # msg+= ','
    # msg+= str(joyY)
    # msg+= '\r\n'
    ser.write(msg.encode('ascii'))
    time.sleep(0.1)

def ReceiveThread():
  while ser.isOpen:
    if ser.in_waiting > 0:
      #print("recived data")
      #c = ser.read(ser.in_waiting)
      c = ser.readline().decode("ascii")
      print(c)
    else:
      time.sleep(0.05)

def LoopbackTest():
  global joyX, joyY
  counter = 0
  joyX = 0
  joyY = 0
  t1 = threading.Thread(target=TransmitThread,args=(counter,))
  t2 = threading.Thread(target=ReceiveThread)
  t1.start()
  t2.start()

  try:
    while True:
      #evdev takes care of polling the controller in a loop
      for event in gamepad.read_loop():
        if event.type == 3: # Let Thumb joystick
          if event.code == 1: # Y axis
            # joyY = ((128 - event.value )/128)
            joyY = (128 - event.value )
            #print(f'Y {joyY }')
          elif event.code == 0: # X axis
            # joyX = ((event.value - 128)/128)
            joyX = (event.value - 128)
            #print(f'X {joyX}')
  except:
      pass

if __name__ == "__main__":
  LoopbackTest()