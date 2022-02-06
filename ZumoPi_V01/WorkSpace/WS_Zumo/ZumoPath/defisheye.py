import cv2
import numpy as np

K=np.array([[495.4835791957818, 0.0, 527.1308746899989], [0.0, 495.608723307695, 379.441690742541], [0.0, 0.0, 1.0]])
D=np.array([[-0.034748616074508305], [-0.020078134379109033], [0.044527094161028694], [-0.030334540004418053]])

img = cv2.VideoCapture(0)
img.set(3, 1024)
img.set(4, 768)
while True:
    success, frame = img.read()
    #h, w = vid.shape[:2]
    map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, (1024, 768), cv2.CV_16SC2)
    undistorted_img = cv2.remap(frame, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
    cv2.imshow("video",undistorted_img)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
img.release()
cv2.destroyAllWindows()