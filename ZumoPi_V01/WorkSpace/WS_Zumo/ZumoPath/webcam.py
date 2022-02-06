import cv2
cap = cv2.VideoCapture(0)
cap.set(3,1024)
cap.set(4,768)

while True:
    suc, img = cap.read()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    cv2.imshow("vid",gray)
    if cv2.waitKey(1) and 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()