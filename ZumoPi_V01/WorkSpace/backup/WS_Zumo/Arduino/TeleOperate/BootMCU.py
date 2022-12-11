#!/usr/bin/python3

from gpiozero import LED
from time import sleep

mcuReset = LED(26)

print("MCU Boot Mode")
mcuReset.on()
sleep(0.5)
mcuReset.off() #rest once
sleep(0.5)
mcuReset.on()
sleep(0.5)
mcuReset.off() #bootloader mode for 8 seconds
sleep(0.5)
mcuReset.on()
print("MCU Boot Mode for 8 seconds")