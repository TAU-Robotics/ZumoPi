#!/usr/bin/python3

from gpiozero import LED
from time import sleep

mcuReset = LED(26)

print("MCU Reset")
mcuReset.on()
sleep(1)
mcuReset.off()
