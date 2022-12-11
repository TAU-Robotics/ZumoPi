#!/usr/bin/python3
import threading
import serial
import time
import tty, sys, termios

# serial connection
ser = serial.Serial('/dev/ttyACM0')  # open serial port

global joyX , joyY , speed

filedescriptors = termios.tcgetattr(sys.stdin)
tty.setcbreak(sys.stdin)

def TransmitThread(counter):
  while ser.isOpen:
    #print("send data")
    global joyX, joyY, speed
    #print(f'joystick {joyX}\t{joyY}')
    #counter+=1
    #msg = 'test ' + str(counter) + '\r\n'
    msg = ''
    msg = str(joyX*speed) + ',' +str(joyY*speed) +'\r\n'
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
  global joyX, joyY, speed
  counter = 0
  joyX = 0
  joyY = 0
  speed = 1
  t1 = threading.Thread(target=TransmitThread,args=(counter,))
  t2 = threading.Thread(target=ReceiveThread)
  t1.start()
  t2.start()

  try:
    while True:
      x = sys.stdin.read(1)[0]
      print(x)
      if x == 'w':
        joyY = 50
        joyX = 0
      elif x == 'a':
        joyY = 0
        joyX = -50
      elif x == 'd':
        joyY = 0
        joyX = 50
      elif x == 's':
        joyY = -50
        joyX = 0
      elif x == '1':
        speed = 1 
      elif x == '2':
        speed = 2
      elif x == '3':
        speed = 3
      elif x == '4':
        speed = 4 
      elif x == '5':
        speed = 5              
      else:
        joyY = 0
        joyX = 0  
  except:
      pass

if __name__ == "__main__":
  LoopbackTest()