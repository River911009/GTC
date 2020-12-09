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
  """shifting image to point1, fill zero in others area

  Args:
      destination_size ([tuple]): [description]
      data ([type]): [description]
      point1 ([tuple]): [description]
      point2 ([tuple]): [description]
      border (bool, optional): [description]. Defaults to False.
      zoom_ratio (int, optional): [description]. Defaults to 1.
  """
  if point2[0]>point1[0] and point2[1]>point1[1]:
    # zoom
    data=cv2.resize(
      src=data,
      dsize=(
        zoom_ratio*data.shape[1],
        zoom_ratio*data.shape[0]
      ),
      interpolation=cv2.INTER_NEAREST
    )

    # split frame from zero to point2
    if data.shape[0]>zoom_ratio*point2[1]:
      data=data[:zoom_ratio*point2[1],:]
    if data.shape[1]>zoom_ratio*point2[0]:
      data=data[:,:zoom_ratio*point2[0]]

  # fill zero after data if point2 is larger than data size
    if data.shape[0]<zoom_ratio*point2[1]:
      nullpxl=np.zeros(
        shape=(
          zoom_ratio*point2[1]-data.shape[0],
          data.shape[1]
        )
      )
      data=np.vstack([data,nullpxl])
    if data.shape[1]<zoom_ratio*point2[0]:
      nullpxl=np.zeros(
        shape=(
          data.shape[0],
          zoom_ratio*point2[0]-data.shape[1]
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
    if data.shape[0]>zoom_ratio*destination_size[1]:
      data=data[:zoom_ratio*destination_size[1],:]
    if data.shape[1]>zoom_ratio*destination_size[0]:
      data=data[:,:zoom_ratio*destination_size[0]]

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


def testFrame(light):
  frame=np.zeros(shape=(22,22),dtype=np.uint8)
  # test frame type A
  # for i in range(11):
  #   for j in range(11):
  #     frame[i*2][j*2]=light
  # test frame type B
  for i in range(22):
    for j in range(22):
      frame[i][j]=i*j*255//484
  # for i in range(frame.size):
  #   frame[i]=(3*i)%256
  # frame=np.reshape(frame,(12,16))
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
      testFrame(255),
      (0,5),
      (12,16),
      True,
      20
    )
    window['__IMAGE__'].update(data=cv2.imencode(ext='.png',img=frame)[1].tobytes())
  window.close()
