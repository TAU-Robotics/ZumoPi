import cv2
cap = cv2.VideoCapture(0)
i=0

while True:
    suc, img = cap.read()
    if (i == 0):
        print(img.shape)
        i=i+1
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    cv2.imshow("vid",gray)
    cv2.waitKey(2)
    break

cap.release()
cv2.destroyAllWindows()