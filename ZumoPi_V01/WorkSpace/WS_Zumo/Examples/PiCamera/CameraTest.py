from time import sleep
from picamera import PiCamera

camera = PiCamera()
camera.resolution = (1024, 768)
# camera.resolution = (2592, 1944)
camera.start_preview()
# Camera warm-up time
sleep(1)
camera.capture('foo5.jpg')
sleep(1)
camera.close()
