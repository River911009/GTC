''' getImgArray

  GTC160 -> *.csv, *.bmp and *.c
  data array are uint8 grey of UVC

  by Riviere @ 9 Dec, 2020

'''

import cv2
import numpy as np
import PySimpleGUI as sg
from pathlib import Path

from numpy.core.fromnumeric import size


IMAGE_SIZE=(160,120)
counter=1

def returnCameraIndexes():
  # checks the first 10 indexes.
  index = 0
  arr = []
  i = 10
  while i > 0:
    cap = cv2.VideoCapture(index)
    if cap.read()[0]:
      arr.append(index)
      cap.release()
    index += 1
    i -= 1
  return arr

cam_list=returnCameraIndexes()
print(cam_list)

window=sg.Window(title='tinyThermalCam',layout=[
  [sg.Text('Camera',size=(10,1)),sg.InputCombo(values=cam_list,size=(20,1),key='CAMERA')],
  [sg.Text('File name',size=(10,1)),sg.Input('IMG0001',size=(20,1),key='FNAME')],
  [sg.Text('Average count:',size=(10,1)),sg.Input('1',size=(20,1),key='FCOUNT')],
  [sg.ProgressBar(max_value=100,size=(30,20),key='PROGRESS')],
  [sg.Button('Capture!',size=(10,1))],
  [sg.Text('',size=(20,5),key='LIST')]
],size=(320,240)).finalize()



pth = Path('./DCIM')
pth.mkdir(exist_ok=True)

def capture(fname,avg):
  camera=cv2.VideoCapture(values['CAMERA'])

  camera.set(cv2.CAP_PROP_CONVERT_RGB,0)
  camera.set(cv2.CAP_PROP_MODE,3)

  # initial camera shape
  # camera.set(cv2.CAP_PROP_FRAME_WIDTH, IMAGE_SIZE[1])
  # camera.set(cv2.CAP_PROP_FRAME_HEIGHT, IMAGE_SIZE[0])

  window['PROGRESS'].UpdateBar(0)
  tmp=np.zeros(shape=(180,160),dtype=np.int16)
  for i in range(avg):
    window['PROGRESS'].UpdateBar(100*(i+1)/avg)
    ret,frame=camera.read()


    print(size(frame))
    if size(frame)==28800:
      shaped=np.reshape(frame,newshape=(IMAGE_SIZE[1]+IMAGE_SIZE[1]//2,IMAGE_SIZE[0]))

      # cv2.imshow('test',shaped)

      print('raw:'+str(frame.shape)+', reshaped:'+str(shaped.shape))
      tmp=np.add(tmp,shaped)
      print(i,shaped[1][1])
    else:
      sg.popup_error('frame size not match!')
  tmp=tmp//avg
  RawFrame=tmp.astype(np.uint8)
  print('a',RawFrame[1][1])

  camera.release()

  with open('./DCIM/'+fname+'.csv','w',encoding='utf-8') as f:
    for y in range(180):
      for x in range(160):
        f.write(str(RawFrame[y][x]))
        f.write(',')
      f.write('\n')
  f.close()

  with open('./DCIM/'+fname+'.c','w',encoding='utf-8') as f:
    f.write('uint8_t image_array[180][160]={\n')
    for y in range(180):
      f.write('  {')
      for x in range(160):
        f.write(str(RawFrame[y][x]))
        if x<159:
          f.write(',')
      if y<179:
        f.write('},\n')
      else:
        f.write('}\n};\n')
  f.close()

  cv2.imwrite('./DCIM/'+fname+'.bmp',RawFrame)

while True:
  event,values=window.read(timeout=100)

  if event==sg.WIN_CLOSED:
    break

  try:
    if int(values['FCOUNT'])<1:
      window['FCOUNT'].update('1')
    elif int(values['FCOUNT'])>256:
      window['FCOUNT'].update('256')
  except():
    window['FCOUNT'].update('1')

  if event=='Capture!':
    if counter<10:
      window['FNAME'].update('IMG000'+str(counter+1))
    if counter>=10:
      window['FNAME'].update('IMG00'+str(counter+1))
    if counter>=100:
      window['FNAME'].update('IMG0'+str(counter+1))
    if counter>=1000:
      window['FNAME'].update('IMG'+str(counter+1))
    counter+=1

    window['LIST'].update(values['FNAME']+' saved!')
    capture(values['FNAME'],int(values['FCOUNT']))

window.close()
