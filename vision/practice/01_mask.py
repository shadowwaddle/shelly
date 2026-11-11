import cv2
import numpy as np

cam = cv2.VideoCapture(0)

frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))

if not cam.isOpened():
    print("Cannot open camera")
    exit()

kernel = np.ones((5, 5), np.uint8)

while True:
    ret, frame = cam.read()

    if not ret:
        print("Can't receive fram. Exiting")
        break

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    lower = np.array([0, 100, 50])
    upper = np.array([10, 255, 255])
    mask = cv2.inRange(hsv, lower, upper)

    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    contours, hierarchy = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        target = max(contours, key=cv2.contourArea)
        if cv2.contourArea(target) > 500:
            x, y, w, h = cv2.boundingRect(target)
            cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

            M = cv2.moments(target)
            cX = int(M["m10"] / M["m00"])
            cY = int(M["m01"] / M["m00"])

            cv2.circle(frame, (cX, cY), 5, (255, 255, 255), -1)
            cv2.line(frame, (frame_width//2, 0), (frame_width//2, frame_height), (255,0,0), 2 )
            error = cX-frame_width//2
            cv2.putText(frame, f"error: {error}", (50, 250), cv2.FONT_HERSHEY_SIMPLEX, 1, (250,0,0), 2, cv2.LINE_AA )
    
    
    cv2.imshow("Frame", frame)   # color image with your box
    cv2.imshow("Mask", mask)     # black-and-white mask

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cam.release()
cv2.destroyAllWindows()