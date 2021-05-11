''' getImgArray

  GTC160 -> *.csv, *.bmp and *.c
  data array are uint8 grey of UVC

  by Riviere @ 9 Dec, 2020

'''

import cv2
import numpy as np
import PySimpleGUI as sg
from pathlib import Path


IMAGE_SIZE=(160,120)
counter=1

window=sg.Window(title='tinyThermalCam',layout=[
  [sg.Text('File name',size=(10,1))],
  [sg.Input('IMG0001',size=(20,1),key='FNAME')],
  [sg.Button('Capture!',size=(10,1))],
  [sg.Text('',size=(20,5),key='LIST')]
],size=(320,240))
camera=cv2.VideoCapture(0)

camera.set(cv2.CAP_PROP_CONVERT_RGB,0)
camera.set(cv2.CAP_PROP_MODE,3)

# initial camera shape
camera.set(cv2.CAP_PROP_FRAME_WIDTH, IMAGE_SIZE[1])
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, IMAGE_SIZE[0])

pth = Path('./DCIM')
pth.mkdir(exist_ok=True)

def capture(fname):
  ret,frame=camera.read()
  RawFrame=np.reshape(frame,newshape=(IMAGE_SIZE[1]+IMAGE_SIZE[1]//2,IMAGE_SIZE[0]))

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

  if event=='Capture!':
    if counter<10:
      window.find('FNAME').update('IMG000'+str(counter+1))
    if counter>=10:
      window.find('FNAME').update('IMG00'+str(counter+1))
    counter+=1

    window.find('LIST').update(values['FNAME']+' saved!')
    capture(values['FNAME'])

window.close()
camera.release()
