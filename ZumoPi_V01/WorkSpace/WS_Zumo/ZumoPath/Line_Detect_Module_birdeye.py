import cv2
import numpy as np
import utlis

def getLaneCurve(img):
    imgthres = utlis.thresholding(img)
    hT , wT , c = img.shape

    points = utlis.valTrackbars()
    imgWarp = utlis.warpImg(imgthres, points, wT, hT)
    imgWarpPoints = utlis.drawPoints(img, points)
    imgthres_resized = cv2.resize(imgthres,(480,360))
    imgWarp_resized = cv2.resize(imgWarp, (480, 360))
    imgWarpPoints_resized = cv2.resize(imgWarpPoints, (480, 360))
    cv2.imshow("thers",imgthres_resized)
    cv2.imshow("warp", imgWarp_resized)
    cv2.imshow("WarpPoint", imgWarpPoints_resized)
    return None


if __name__ == '__main__':
    #cap = cv2.VideoCapture('vid1.mp4')
    intialTracbarVals =  [179,651,78,768]
    utlis.initializeTrackbars(intialTracbarVals)
    K = np.array([[495.4835791957818, 0.0, 527.1308746899989], [0.0, 495.608723307695, 379.441690742541], [0.0, 0.0, 1.0]])
    D = np.array([[-0.034748616074508305], [-0.020078134379109033], [0.044527094161028694], [-0.030334540004418053]])
    cap = cv2.VideoCapture(0)
    cap.set(3, 1024)
    cap.set(4, 768)
    #intialTrackBarVals = &  # 91;102, 80, 20, 214 ]
    #utlis.initializeTrackbars(intialTrackBarVals)
    frameCounter = 0
    while True:
        frameCounter += 1
        if cap.get(cv2.CAP_PROP_FRAME_COUNT) == frameCounter:
            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
            frameCounter = 0

        success, img = cap.read()
        #img = cv2.resize(img, (480, 240))
        map1, map2 = cv2.fisheye.initUndistortRectifyMap(K, D, np.eye(3), K, (1024, 768), cv2.CV_16SC2)
        undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
        getLaneCurve(undistorted_img)
        #print(curve)
        undistorted_img_resized = cv2.resize(undistorted_img, (480, 360))
        #cv2.imshow("video", undistorted_img_resized)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        #cv2.imshow('Vid',img)
        #cv2.waitKey(1)

cap.release()
cv2.destroyAllWindows()