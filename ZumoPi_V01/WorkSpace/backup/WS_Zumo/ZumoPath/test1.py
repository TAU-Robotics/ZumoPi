import cv2

img = cv2.imread("foo4.jpg")
cv2.imshow("undistorted", img)
cv2.waitKey(0)