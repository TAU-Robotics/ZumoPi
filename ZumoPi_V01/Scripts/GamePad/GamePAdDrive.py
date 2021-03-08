#!/usr/bin/env python3
import evdev
from evdev import InputDevice, categorize, ecodes

#create car object
#car = NvidiaRacecar()

#creates object 'gamepad' to store the data
#you can call it whatever you like
gamepad = InputDevice('/dev/input/event0')

#prints out device info at start
print(gamepad)

#initialize car parameters
#car.steering_offset=0
#car.steering = 0
#car.throttle_gain = 1

#evdev takes care of polling the controller in a loop
for event in gamepad.read_loop():
	#filters by event type
	if event.type == 3: # Let Thumb joystick
		if event.code == 1: # Y axis
	    		#car.throttle = ((128 - event.value )/128)
	    		print(f'Y {event.value}')
		elif event.code == 0: # X axis
                        #car.steering = ((event.value - 128)/128)
                        print(f'X {event.value}')
