  ''' GTC Thermal Camera Analysis 
    Version
      1.0
    Date
      14 Aug, 2020
    Author
      Riviere
    Descript
      This App is designed to camera hardware tuning
  '''

import PySimpleGUI as sg
import numpy as np
import cv2 as cv
import time
import sys
import glob
import serial


def serial_ports():
    """ Lists serial port names
 
        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


########################################
# Global varables
########################################
# static variables
WINDOW_TITLE='GTC Thermal Camera'
SERIAL_PORTS=serial_ports()
CAMERA_LIST=['0','1']
PGA_GAIN=['1','2','4','8']
IMAGE_SIZE=(160,120)
SCREEN_SIZE=(480,360)
FORM_ITEM_WIDTH=10
AUTO_SCALE=True

# Variables
DebugMode=False
RawFrame=np.zeros(shape=(120,160),dtype=np.uint8)
ErrorFlag=0
CameraId=0
DefectivePixel=0

# connection panel includes camera and serial selector
ConnectionLayout=[
  # Camera selector
  sg.Text(
    text='Camera',
    size=(FORM_ITEM_WIDTH,1)
  ),
  sg.InputCombo(
    values=CAMERA_LIST,
    default_value=0,
    size=(FORM_ITEM_WIDTH,1),
    key='__CAMERA__'
  ),
  # Serial selector
  sg.Text(
    text='Serial port',
    size=(FORM_ITEM_WIDTH,1)
  ),
  sg.InputCombo(
    values=SERIAL_PORTS,
    default_value=None,
    size=(FORM_ITEM_WIDTH,1),
    key='__SERIAL__'
  )
]

# screen panel includes video and graph screen
ScreenLayout=[
  # video screen
  sg.Image(
    filename='',
    size=SCREEN_SIZE,
    key='__IMAGE__'
  ),
  # graph screen
  sg.Image(
    filename='',
    size=SCREEN_SIZE,
    key='__GRAPH__'
  )
]

# control panel includes FW controllers
ControlLayout=[
  [
    sg.Text(
      text='PGA gain',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.InputCombo(
      values=PGA_GAIN,
      default_value=PGA_GAIN[0],
      size=(FORM_ITEM_WIDTH,1),
      key='__PGAGAIN__'
    )
  ],
  [
    sg.Text(
      text='VBPXL',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.InputText(
      default_text='0',
      size=(FORM_ITEM_WIDTH,1),
      key='__VBPXL__'
    )
  ],
  [
    sg.Text(
      text='VBREF',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.InputText(
      default_text='0',
      size=(FORM_ITEM_WIDTH,1),
      key='__VBREF__'
    )
  ],
  [
    sg.Text(
      text='VBLINE',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.InputText(
      default_text='0',
      size=(FORM_ITEM_WIDTH,1),
      key='__VBLINE__'
    )
  ],
  [
    sg.Text(
      text='VBOA',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.InputText(
      default_text='0',
      size=(FORM_ITEM_WIDTH,1),
      key='__VBOA__'
    )
  ]
]

# calculate panel shows analysis datas
CalculateLayout=[
  [
    sg.Text(
      text='Defective pixel',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.Text(
      size=(FORM_ITEM_WIDTH,1),
      key='__DEPXL__'
    )
  ],
  [
    sg.Text(
      text='Std Deviation',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.Text(
      size=(FORM_ITEM_WIDTH,1),
      key='__STDEV__'
    )
  ],
  [
    sg.Text(
      text='Frame Rate',
      size=(FORM_ITEM_WIDTH,1)
    ),
    sg.Text(
      size=(FORM_ITEM_WIDTH,1),
      key='__FRATE__'
    )
  ],
  [sg.Text('',key='__TEST__')],
  [sg.Text('')]
]

# graph type panel includes graph screen function controller
GraphTypeLayout=[
  [
    sg.Radio(
      text='Histogram',
      group_id='GRAPH_TYPE',
      default=True,
      size=(FORM_ITEM_WIDTH*2,1),
      key='__HIST__'
    )
  ],
  [
    sg.Radio(
      text='Line',
      group_id='GRAPH_TYPE',
      size=(FORM_ITEM_WIDTH,1),
      key='__LINE__'
    )
  ],
  [
    sg.Radio(
      text='RAW',
      group_id='GRAPH_TYPE',
      size=(FORM_ITEM_WIDTH,1),
      key='__RAW__',
      disabled=True
    )
  ],
  [sg.Text('')],
  [sg.Text('')]
]

# main window layout
WindowLayout=[
  [sg.Frame(title='Connection',layout=[ConnectionLayout])],
  [sg.Frame(title='Screen',layout=[ScreenLayout])],
  [
    # sg.Text(text='',size=(FORM_ITEM_WIDTH*3,1)),
    sg.Frame(title='Control Panel',layout=ControlLayout),
    sg.Frame(title='Calculate Panel',layout=CalculateLayout),
    sg.Frame(title='Graph type',layout=GraphTypeLayout)
  ],
  [sg.Text(text='',size=(FORM_ITEM_WIDTH*11,1)),sg.Button(button_text='QUIT',size=(FORM_ITEM_WIDTH,1))]
]


########################################
# Sub function
########################################
# debug functions
def debugRawDataPrint(frame,x,y,length):
  for y_axis in range(length):
    for x_axis in range(length):
      print('%4d'%(frame[0][160*(x+x_axis)+(y+y_axis)]),end='')
    print('')

# calculate and draw histrogram
def drawHist(
    data,
    shape=(256,256),
    auto_scale=True,
    background=(255,255,255),
    color=(128,128,128)
  ):
  # create graph
  histGraph=np.zeros((shape[1],shape[0],3),dtype=np.uint8)
  histGraph[:]=background
  graph=np.zeros((20,shape[0],3),dtype=np.uint8)
  graph[:]=background

  # convert histogram
  #--- fine convertion [0:556+38,125:18580,255:26]
  barArray=cv.calcHist(images=[data],channels=[0],mask=None,histSize=[shape[0]],ranges=[0,256])
  #---  bad convertion [0:524,125:18635,255:41]
  # histGraph=np.zeros(256,dtype=int)
  # for index in range(data.size):
  #   histGraph[data[index//160][index%120]%256]+=1

  # find highest value and argument
  highest_argument=np.argmax(barArray)
  highest=int(np.max(barArray))

  # automatic scale
  if highest>shape[1] and auto_scale:
    scale=highest/shape[1]
  else:
    scale=1

  # draw each bar
  for bar in range(int(barArray.size)):
    # scale
    if AUTO_SCALE:
      barArray[bar]/=scale

    if barArray[bar]>0:
      cv.line(
        img=histGraph,
        pt1=(bar,shape[1]-1),
        pt2=(bar,shape[1]-(
            int(np.clip(barArray[bar],0,shape[1]))
          )
        ),
        color=color
      )

  # display height of highest bar
  if highest>shape[0] or auto_scale:
    cv.putText(
      img=graph,
      text='Maximum['+str(int(highest))+']',
      org=(np.clip(int(highest_argument-50),0,250),15),
      fontFace=cv.FONT_HERSHEY_SIMPLEX,
      fontScale=0.5,
      color=(0,0,0)
    )

  return np.vstack([graph,histGraph])

# draw line
def drawLine(
    data,
    shape=(160,256),
    y_axis=100,
    background=(255,255,255),
    color=(128,128,128)
  ):
  # create graph
  lineGraph=np.zeros((shape[1],shape[0],3),dtype=np.uint8)
  titleBar=np.zeros((20,shape[0],3),dtype=np.uint8)
  lineGraph[:]=background
  titleBar[:]=background

  # draw each bar
  for bar in range(shape[0]-1):
    cv.line(
      img=lineGraph,
      pt1=(bar,shape[1]-data[y_axis][bar]),
      pt2=(bar+1,shape[1]-data[y_axis][bar+1]),
      color=color
    )

  # draw y-axis level
  cv.putText(
    img=titleBar,
    text='Level['+str(y_axis)+']',
    org=(50,15),
    fontFace=cv.FONT_HERSHEY_SIMPLEX,
    fontScale=0.4,
    color=(0,0,0)
  )

  return np.vstack([titleBar,lineGraph]) 

# display image onto two screen panel
def screenRefresh(
    screen,
    type,
    frame,
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

  # refresh a screen
  window[screen].update(
    data=cv.imencode(ext='.png',img=frame)[1].tobytes()
  )


########################################
# Main initial function
########################################
window=sg.Window(WINDOW_TITLE).Layout(WindowLayout).finalize()

camera=cv.VideoCapture(int(CAMERA_LIST[CameraId]))

# initial to YUV grab mode
camera.set(cv.CAP_PROP_CONVERT_RGB,0)
camera.set(cv.CAP_PROP_MODE,3)

# check the camera size, 160x160 in debug mode
if(camera.get(cv.CAP_PROP_FRAME_WIDTH)==160
  and camera.get(cv.CAP_PROP_FRAME_HEIGHT)==160):
  DebugMode=True
DebugMode=True

# initial camera shape
camera.set(cv.CAP_PROP_FRAME_WIDTH, IMAGE_SIZE[1])
camera.set(cv.CAP_PROP_FRAME_HEIGHT, IMAGE_SIZE[0])

# initial gui datas
window['__DEPXL__'].update('0')
window['__STDEV__'].update('0')

if DebugMode:
  window['__RAW__'].update(disabled=False)
  window.set_title(WINDOW_TITLE+' (Debug Mode)')

########################################
# Loop function
########################################
previousTime=0
previousFrame=0
frameRate=0

while True:
  event,values=window.read(timeout=20)
  if event=='QUIT' or event==sg.WIN_CLOSED:
    break

  ret,frame=camera.read()

  print('raw frame size:',frame.size,'\tshape:',frame.shape)
  RawFrame=np.reshape(frame,newshape=(IMAGE_SIZE[1]+IMAGE_SIZE[1]//2,IMAGE_SIZE[0]))
  print(RawFrame[120][0],RawFrame[120][1],RawFrame[120][2],RawFrame[120][3],RawFrame[120][4],RawFrame[120][5])

  if previousTime==time.localtime().tm_sec:
    if not previousFrame==RawFrame[120][5]:
      frameRate+=1
      previousFrame=RawFrame[120][5]

  RawFrame=RawFrame[:120][:]
  print('after reshape\nimg shape:',RawFrame.shape)


  #---------------------------
  # try to add disconnect detect
  #---------------------------
  # if ret==False:
  #   camera.release()
  #   if ErrorFlag==0:
  #     if 'YES'==sg.popup('Camera disconnected!','Reconnect?',title='Error',button_type=sg.POPUP_BUTTONS_YES_NO):
  #       print(0)

  #   ErrorFlag=1

  # else:
  #---------------------------

  screenRefresh('__IMAGE__','',cv.resize(src=RawFrame,dsize=SCREEN_SIZE,interpolation=cv.INTER_LINEAR))

  if not previousTime==time.localtime().tm_sec:
    window['__FRATE__'].update(str(frameRate))
    frameRate=0

  if   values['__HIST__']==True:
    screenRefresh('__GRAPH__','__HISTROGRAM__',drawHist(RawFrame))
  elif values['__LINE__']==True:
    screenRefresh('__GRAPH__','__LINE__',drawLine(RawFrame,y_axis=20))
  # elif values['__RAW__']==True:


  if not CameraId==int(values['__CAMERA__']):
    camera.release()
    CameraId=int(values['__CAMERA__'])
    camera=cv.VideoCapture(int(CAMERA_LIST[CameraId]))

  # sync system time
  previousTime=time.localtime().tm_sec

########################################
# Memery recycle
########################################
camera.release()
window.close()
