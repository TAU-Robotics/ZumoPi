from picamera import PiCamera
import time
camera = PiCamera()
time.sleep(2)
camera.capture("/home/pi/Pictures/img35.jpg")
print("Done.")