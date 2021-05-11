''' bin2img

  binary -> display on GUI

  by Riviere @ 16 April, 2021

'''

import os
import cv2 as cv
import numpy as np
import PySimpleGUI as sg

########################################
# Var
########################################
width=160
height=120

layout=[
  sg.Frame(title='',layout=[
      [
        sg.Text(text='Path:'),
        sg.Input(size=(24,1),key='__PATH__')],
      [
        sg.Text(text='W:'),
        sg.Input(default_text='160',size=(10,1),key='__WIDTH__'),
        sg.Text(text='H:'),
        sg.Input(default_text='120',size=(10,1),key='__HEIGHT__')
      ],
      [sg.Listbox([],size=(30,10),key='__LIST__')],
      [
        sg.Button('EXIT',size=(24,1),key='__CVT__'),
        sg.Button('?',size=(2,1),key='__HELP__')
      ]
    ],border_width=0),
    sg.Image(filename='',background_color='black',key='__IMAGE__',size=(2*width,2*height))
]

window=sg.Window(title="Blackrice ask me to decode bin to img",layout=[layout]).finalize()

########################################
# Sub func
########################################
# display image onto two screen panel
def screenRefresh(
    screen,
    type,
    frame,
    level=120,
    shape=(480,360),
    color=(0,0,0),
    background=(255,255,255)
  ):
  if   type=='__HISTROGRAM__':
    unit=255
  elif type=='__LINE__':
    unit=159
  else:
    unit=0

  if not unit==0:
    # create margins and variable
    bottom=np.zeros((20,shape[0],3),dtype=np.uint8)
    margin=np.zeros((shape[1]-20,20,3),dtype=np.uint8)
    bottom[:]=background
    margin[:]=background

    # draw scales
    cv.line(bottom,(20,0),(shape[0]-20,0),color)
    cv.putText(bottom,'0',(15,15),cv.FONT_HERSHEY_SIMPLEX,0.5,color)
    cv.putText(bottom,str(unit//2),(223,15),cv.FONT_HERSHEY_SIMPLEX,0.5,color)
    cv.putText(bottom,str(unit),(445,15),cv.FONT_HERSHEY_SIMPLEX,0.5,color)

    # marge
    frame=cv.resize(src=frame,dsize=(shape[0]-40,shape[1]-20),interpolation=cv.INTER_LINEAR)
    frame=np.hstack([margin,frame,margin])
    frame=np.vstack([frame,bottom])

    if values['__LINE__']==True:
      cv.line(frame,(0,level*3),(10,level*3),(255,0,0))

  # refresh a screen
  window[screen].update(
    data=cv.imencode(ext='.png',img=frame)[1].tobytes()
  )

def decode(f):
  img=np.zeros(shape=(width*height),dtype=np.uint8)
  if f:
    with open(f[0],"rb") as array:
      i=0
      tmp=array.read(1)
      while tmp and i<width*height:
        img[i]=int.from_bytes(tmp,byteorder="big",signed=False)
        tmp=array.read(1)
        i+=1
      
      array.close()
  return(np.reshape(a=img,newshape=(width,height)))

########################################
# Init
########################################
pset_list=[a for a in os.listdir('./')]
window.FindElement('__LIST__').Update(pset_list)
window.FindElement('__PATH__').Update(os.getcwd())


########################################
# Main loop
########################################

while True:
  event,values=window.read(timeout=20)

  if event in (sg.WIN_CLOSED,'QUIT'):
    break

  if os.getcwd()!=values['__PATH__']:
    try:
      os.chdir(values['__PATH__'])
      pset_list=[a for a in os.listdir('./')]
      window.FindElement('__LIST__').Update(pset_list)
    except:
      pass

  if event=='__HELP__':
    sg.popup("Lorem Ipsum is simply dummy text of the printing and\ntypesetting industry. Lorem Ipsum has been the industry's\nstandard dummy text ever since the 1500s, when an unknown\nprinter took a galley of type and scrambled it to make a\ntype specimen book. It has survived not only five centuries,\nbut also the leap into electronic typesetting, remaining\nessentially unchanged. It was popularised in the 1960s\nwith the release of Letraset sheets containing Lorem\nIpsum passages, and more recently with desktop publishing\nsoftware like Aldus PageMaker including versions of Lorem Ipsum.\n\nI think you should more understood with those tutorial, if not... go ask Riviere again with some drink and snack!!!",title='Help centre',)

  if event=='__CVT__':
    width=int(values['__WIDTH__'])
    height=int(values['__HEIGHT__'])
    screenRefresh('__IMAGE__','',cv.resize(src=decode(values['__LIST__']),dsize=(2*height,2*width),interpolation=cv.INTER_LINEAR))

########################################
# Memery recycle
########################################
window.close()