''' getTempArray

  GTC160 -> data.csv
  data array are float temperature

  by Riviere @ 21 Jan, 2021

'''

import cv2
import numpy


# rawImg is senser raw data array
rawImg  = numpy.zeros(shape=(120,160))
# tempImg is temperature array
tempImg = numpy.zeros(shape=(120,160))



camera = cv2.VideoCapture(0)

# disable convert to RGB
camera.set(cv2.CAP_PROP_CONVERT_RGB,0)

# read one frame image raw data from camera
ret, image = camera.read()

# the UVC imcoming data is (1*28800)
# should be reshape to (180*160)
rawImg = numpy.reshape(image,newshape=(180,160))


tempValue    = rawImg[120][0]*256+rawImg[120][1]
sensorOffset = rawImg[120][2]*256+rawImg[120][3]
tempRatio    = rawImg[120][4]


# turn unsigned int 16 sensorOffset to signed int 16
if sensorOffset > 32767:
  sensorOffset -= 65536

rawImg=rawImg[:120][:]


for y in range(120):
  for x in range(160):
    tempImg[y][x]=((rawImg[y][x]-sensorOffset)/tempRatio)+(tempValue/100)



with open('data.csv','w') as f:
  for y in range(120):
    for x in range(160):
      f.write('%.2f'%tempImg[y][x])
      f.write(',')
    f.write('\n')
  f.close()


cv2.imshow('GTC pixel value',rawImg)


res = cv2.waitKey(0)

camera.release()




# import cv2
# from tqdm import trange


# cap = cv2.VideoCapture(0)
# f = open('results.txt', 'w')

# frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))

# for i in trange(100, unit=' frames', leave=False, dynamic_ncols=True, desc='Calculating blur ratio'):
# 	ret, frame = cap.read()
# 	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
# 	fm = cv2.Laplacian(gray, cv2.CV_64F).var()

# 	# Sample quality bar. Parameters adjusted manually to fit horizontal image size
# 	cv2.rectangle(frame, (0, 1080), (int(fm*1.6), 1040), (0,0,255), thickness=cv2.FILLED)

# 	im = cv2.resize(frame, None,fx=0.5, fy=0.5, interpolation = cv2.INTER_CUBIC)
# 	cv2.imshow("Output", im)

# 	f.write(str(fm)+'\r')

# 	k = cv2.waitKey(1) & 0xff
# 	if k == 27:
# 		break