import cv2
assert cv2.__version__[0] == '4', 'The fisheye module requires opencv version >= 4.0.0'
import numpy as np
import sys
import os
import glob

import getopt

DIM=(1024, 768)
K=np.array([[495.4835791957818, 0.0, 527.1308746899989], [0.0, 495.608723307695, 379.441690742541], [0.0, 0.0, 1.0]])
D=np.array([[-0.034748616074508305], [-0.020078134379109033], [0.044527094161028694], [-0.030334540004418053]])

def undistort(img_path):
    #img = cv2.imread(img_path)
   # h,w = img.shape[:2]
    map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, DIM, cv2.CV_16SC2)
    undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
    cv2.imshow("undistorted", undistorted_img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
if __name__ == '__main__':
    img = cv2.imread("foo4.jpg")
    #for p in sys.argv[1:]:
    undistort(img)