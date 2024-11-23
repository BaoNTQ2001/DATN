import cv2
import urllib.request
import numpy as np
import os
import face_recognition
import requests

# step 1 load ảnh từ kho ảnh nhận dạng

path = r'C:\Users\Acer\Desktop\Python_OpenCV\pic2'  # đường dẫn vào kho ảnh nhận dạng
url = 'http://192.168.1.100/cam-lo.jpg'

images = []

classNames = []
myList = os.listdir(path)
print(myList)
for cl in myList:
    print(cl)
    curImg = cv2.imread(f"{path}/{cl}")  # pic2/Donal Trump.jpg
    images.append(curImg)
    classNames.append(os.path.splitext(cl)[0])
    # splitext sẽ tách path ra thành 2 phần, phần trước đuôi mở rộng và phần mở rộng
print(len(images))
print(classNames)
# step encoding
def mahoa(images):
    encodeList = []
    for img in images:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)  # BGR được chuyển đổi sang RGB
        encode = face_recognition.face_encodings(img)[0]
        encodeList.append(encode)
    return encodeList


encodeListKnow = mahoa(images)
print("ma hoa thanh cong")
print(len(encodeListKnow))

# khởi dộng webcam
# cap = cv2.VideoCapture(0)

while True:
    # ret, frame= cap.read()
    img_resp = urllib.request.urlopen(url)
    imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
    img = cv2.imdecode(imgnp, -1)

    imgS = cv2.resize(img, (0, 0), None, fx=0.5, fy=0.5)
    imgS = cv2.cvtColor(imgS, cv2.COLOR_BGR2RGB)

    # xác định vị trí khuôn mặt trên cam và encode hình ảnh trên cam
    facecurFrame = face_recognition.face_locations(imgS)  # lấy từng khuôn mặt và vị trí khuôn mặt hiện tại
    encodecurFrame = face_recognition.face_encodings(imgS)

    for encodeFace, faceLoc in zip(encodecurFrame, facecurFrame):  # lấy từng khuôn mặt và vị trí khuôn mặt hiện tại theo cặp
        matches = face_recognition.compare_faces(encodeListKnow, encodeFace)
        faceDis = face_recognition.face_distance(encodeListKnow, encodeFace)
        print(faceDis)
        matchIndex = np.argmin(faceDis)  # đẩy về index của faceDis nhỏ nhất


        if faceDis[matchIndex] < 0.60:
            name = classNames[matchIndex].upper()
        else:
            name = "Unknow"

        # print tên lên frame
        y1, x2, y2, x1 = faceLoc
        y1, x2, y2, x1 = y1*2, x2*2, y2*2, x1*2
        cv2.rectangle(img, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(img, name, (x2, y2), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 2)
        if name != "Unknow":
            requests.get('http://192.168.1.100/opendoor')

    cv2.imshow('Web cam', img)
    if cv2.waitKey(1) == ord("q"):  # độ trễ 1/1000s , nếu bấm q sẽ thoát
        break
#cap.release()  # giải phóng camera
cv2.destroyAllWindows()  # thoát tất cả các cửa sổ
