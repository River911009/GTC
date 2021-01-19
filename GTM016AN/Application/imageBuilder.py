import cv2
import numpy as np
import PySimpleGUI as sg


def imageShift(
  destination_size,
  data,
  point1,
  point2,
  border=False,
  zoom_ratio=1
):
  '''
  # imageShift shifting image to point1, fill zero in others area

  ## Arguments:
      destination_size (tuple): destination image size (x-axis, y-axis).
      data (tuple): source image (y-axis, x-axis).
      point1 (tuple): left top corner of source image on destination image (x-axis, y-axis).
      point2 (tuple): right bottom corner of source image on destination image (x-axis, y-axis).
      border (bool, optional): border width of source image. Defaults to False.
      zoom_ratio (int, optional): zoom ratio of source image. Defaults to 1.

  ## Returns:
      tuple: 2D tuple image frame.
  '''
  if point2[0]>point1[0] and point2[1]>point1[1]:
    # split frame from zero to point2
    data=data[:(point2[1]-point1[1]),:(point2[0]-point1[0])]

    # zoom
    data=cv2.resize(
      src=data,
      dsize=(
        zoom_ratio*data.shape[1],
        zoom_ratio*data.shape[0]
      ),
      interpolation=cv2.INTER_NEAREST
    )

    # fill zero after data if point2 is larger than data size
    if data.shape[0]<zoom_ratio*(point2[1]-point1[1]):
      nullpxl=np.zeros(
        shape=(
          zoom_ratio*(point2[1]-point1[1])-data.shape[0],
          data.shape[1]
        )
      )
      data=np.vstack([data,nullpxl])
    if data.shape[1]<zoom_ratio*(point2[0]-point1[0]):
      nullpxl=np.zeros(
        shape=(
          data.shape[0],
          zoom_ratio*(point2[0]-point1[0])-data.shape[1]
        )
      )
      data=np.hstack([data,nullpxl])

    # drawBorder on data frame
    if border:
      cv2.rectangle(
        img=data,
        pt1=(0,0),
        pt2=(
          data.shape[1]-1,
          data.shape[0]-1
        ),
        color=255
      )

    # shift to point1, fill zeros before data
    nullpxl=np.zeros(
      shape=(
        data.shape[0],
        zoom_ratio*point1[0]
      ),
      dtype=np.uint8
    )
    data=np.hstack([nullpxl,data])
    nullpxl=np.zeros(
      shape=(
        zoom_ratio*point1[1],
        data.shape[1]
      ),
      dtype=np.uint8
    )
    data=np.vstack([nullpxl,data])

    # split frame from zero to destination_size
    data=data[:zoom_ratio*destination_size[1],:zoom_ratio*destination_size[0]]

    # fill zero after data if destination_size is larger than data size
    if data.shape[0]<zoom_ratio*destination_size[1]:
      nullpxl=np.zeros(
        shape=(
          zoom_ratio*destination_size[1]-data.shape[0],
          data.shape[1]
        )
      )
      data=np.vstack([data,nullpxl])
    if data.shape[1]<zoom_ratio*destination_size[0]:
      nullpxl=np.zeros(
        shape=(
          data.shape[0],
          zoom_ratio*destination_size[0]-data.shape[1]
        )
      )
      data=np.hstack([data,nullpxl])
    return(data)


def testFrame(
  shape,
  brightness=128,
  type=0
):
  '''testFrame generate 8-bits test frame 

  Arguments:
    shape (tuple): frame size (y-axis, x-axis).
    brightness (uint8): brightness from 0 to 255. Defaults to 128.
    type (int): test frame type (type: 0, 1 and 2). Defaults to 0.

  Returns:
    tuple: 2D tuple image frame.
  '''
  frame=np.zeros(shape=shape,dtype=np.uint8)

  if type==0:
  # test frame type 0
    for i in range((shape[0]+1)//2):
      for j in range((shape[1]+1)//2):
        frame[i*2][j*2]=brightness

  elif type==1:
  # test frame type 1
    for i in range(shape[0]):
      for j in range(shape[1]):
        frame[i][j]=i*j*brightness//(shape[0]*shape[1])

  elif type==2:
  # test frame type 2 (for calibration bar)
    for i in range(frame.shape[1]):
      frame[0][i]=(brightness*i)//frame.shape[1]

  return(frame)


# ---------------------------------------
if __name__ == '__main__':
  window=sg.Window('test mode',[[sg.Image(filename='',background_color='red',key='__IMAGE__')]])
  while True:
    event,values=window.read(timeout=500)
    if event==sg.WINDOW_CLOSED:
      break
    # drawGraph
    frame=imageShift(
      (22,22),
      testFrame((5,7),type=0),
      (10,2),
      (20,8),
      True,
      20
    )
    window['__IMAGE__'].update(data=cv2.imencode(ext='.png',img=frame)[1].tobytes())
  window.close()
