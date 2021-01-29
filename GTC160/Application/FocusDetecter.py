import cv2
import numpy as np
import PySimpleGUI as sg


halfClick=False
maxi=0


def checkRange(data):
  if int(data['__X1__'])>int(data['__X2__']):
    window.find('__X1__').update(int(data['__X2__'])-1)

def get_focus(x):
  print('test')


layout=[
  [
    sg.Frame(
      title='Range',
      layout=[
        [
          sg.Text(text='X1:',size=(10,1)),
          sg.Slider(range=(0, 119), orientation='h', size=(30, 10), default_value=60,key='__X1__')
        ],
        [
          sg.Text(text='Y1:',size=(10,1)),
          sg.Slider(range=(0, 119), orientation='h', size=(30, 10), default_value=40,key='__Y1__')
        ],
        [
          sg.Text(text='X2:',size=(10,1)),
          sg.Slider(range=(0, 159), orientation='h', size=(30, 10), default_value=100,key='__X2__')
        ],
        [
          sg.Text(text='Y2:',size=(10,1)),
          sg.Slider(range=(0, 119), orientation='h', size=(30, 10), default_value=80,key='__Y2__')
        ]
      ]
    ),
    sg.Frame(
      title='Result of Range',
      layout=[
        [
          sg.Text('Average:',(10,2)),
          sg.Text('0',(10,2),key='__AVG__')
        ],
        [
          sg.Text('STD:',(10,2)),
          sg.Text('0',(10,2),key='__STD__')
        ],
        [
          sg.Text('Grade:',(10,2)),
          sg.Text('0',(10,2),key='__GDE__')
        ],
        [
          sg.Text('MAX:',(10,1)),
          sg.Text('0',(10,1),key='__MAX__')
        ]
      ]
    ),
    sg.Button('FOCUS')
  ],
  [
    sg.Image(filename='',key='__RAW__'),
    sg.Image(filename='',key='__IMAGE__')
  ]
]



window=sg.Window("Focus Detecter").Layout(layout)
camera=cv2.VideoCapture(0)

# initial to YUV gray mode
camera.set(cv2.CAP_PROP_CONVERT_RGB,0)
camera.set(cv2.CAP_PROP_MODE,3)
########################################
# Loop function
########################################
while True:
  event,values=window.read(timeout=100)

  if event in (sg.WIN_CLOSED,):
    break

  # checkRange(values)

  ret,frame=camera.read()
  RawFrame=np.reshape(a=frame,newshape=(180,160))
  RawFrame=RawFrame[:120][:]

  OUT=RawFrame.copy()

  cv2.rectangle(
    img=OUT,
    pt1=(int(values['__X1__']),int(values['__Y1__'])),
    pt2=(int(values['__X2__']),int(values['__Y2__'])),
    color=255,
    thickness=1
  )

  cal=RawFrame[int(values['__Y1__']):int(values['__Y2__'])+1,int(values['__X1__']):int(values['__X2__'])+1]

  avg=np.average(cal)
  std=np.std(cal)

  std_range=np.zeros(shape=(int(values['__Y2__'])-int(values['__Y1__'])+1,int(values['__X2__'])-int(values['__X1__'])+1))

  for y in range(int(values['__Y2__'])-int(values['__Y1__'])+1):
    for x in range(int(values['__X2__'])-int(values['__X1__'])+1):
      std_range[y][x]=(cal[y][x]-avg)/std
      # std_range[y][x]*=100

  grad_x=cv2.Sobel(std_range, cv2.CV_16S, 1, 0, ksize=3, scale=1, delta=0, borderType=cv2.BORDER_DEFAULT)
  # Gradient-Y
  # grad_y = cv.Scharr(gray,ddepth,0,1)
  grad_y=cv2.Sobel(std_range, cv2.CV_16S, 0, 1, ksize=3, scale=1, delta=0, borderType=cv2.BORDER_DEFAULT)

  # abs_grad_x=cv2.convertScaleAbs(grad_x)
  # abs_grad_y=cv2.convertScaleAbs(grad_y)
  # fm = cv2.addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0)
  sqr_grad_x=gr
  

  window.find('__AVG__').update('%.4f'%avg)
  window.find('__STD__').update('%.4f'%std)
  window.find('__GDE__').update(np.max(fm))

  if event=='FOCUS':
    if maxi<np.max(fm):
      maxi=np.max(fm)
    if maxi==np.max(fm):
      maxi=0
      print('GOT!!')
    window.find('__MAX__').update(maxi)


  window['__IMAGE__'].update(
      data=cv2.imencode(
      ext='.png',
      img=cv2.resize(src=cal,dsize=((int(values['__X2__'])-int(values['__X1__'])+1)*4,(int(values['__Y2__'])-int(values['__Y1__'])+1)*4),
      interpolation=cv2.INTER_NEAREST)
    )[1].tobytes()
  )
  window['__RAW__'].update(
    data=cv2.imencode(
      ext='.png',
      img=cv2.resize(src=OUT,dsize=(640,480),
      interpolation=cv2.INTER_NEAREST)
    )[1].tobytes()
  )

########################################
# Memery recycle
########################################
camera.release()
window.close()
