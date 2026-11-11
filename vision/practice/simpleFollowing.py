import cv2
import numpy as np

cam = cv2.VideoCapture(0)

if not cam.isOpened():
    print("Cannot open camera")
    exit()

frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))

kernel = np.ones((9, 9), np.uint8)


while True:

    #Getting initial image
    ret, frame = cam.read()
    if not ret:
        print("Cannot get image")
        exit()

    #Masking and cleaning the image
    frame = cv2.GaussianBlur(frame, (5, 5), 0)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    lower1 = np.array([0,   120, 70]);  upper1 = np.array([8,   255, 255])
    lower2 = np.array([170, 120, 70]);  upper2 = np.array([179, 255, 255])
    mask = cv2.bitwise_or(cv2.inRange(hsv, lower1, upper1), cv2.inRange(hsv, lower2, upper2))

    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    #Get contours and create box around the largest one
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        target = max(contours, key = cv2.contourArea)
        if cv2.contourArea(target) > 500:
             x, y, w, h = cv2.boundingRect(target)
             cv2.rectangle(frame, (x,y), (x+w, y+h), (0,255,0), 2)
            
             M = cv2.moments(target)
             if M["m00"] == 0:
                 continue
             cX = int(M["m10"] / M["m00"])
             cY = int(M["m01"] / M["m00"])

             cv2.circle(frame, (cX, cY), 5, (255, 255, 255), -1)
             cv2.line(frame, (frame_width//2, 0), (frame_width//2, frame_height), (255,0,0), 2 )
             error = cX-frame_width//2
             cv2.putText(frame, f"error: {error}", (50, 250), cv2.FONT_HERSHEY_SIMPLEX, 1, (250,0,0), 2, cv2.LINE_AA )
        


    cv2.imshow("Mask", mask)
    cv2.imshow("Frame", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cam.release()
cv2.destroyAllWindows()