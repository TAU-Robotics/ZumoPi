import tty, sys, termios
# https://www.circuitbasics.com/how-to-detect-keyboard-and-mouse-inputs-on-a-raspberry-pi/

filedescriptors = termios.tcgetattr(sys.stdin)
tty.setcbreak(sys.stdin)
x = 0
while 1:
  x = sys.stdin.read(1)[0]
  #print("You pressed",x)
  print(x)
  #if x == "D":
  #  print("If condition is met")
termios.tcsetattr(sys.stdin, termios.TCSADRAIN,filedescriptors)