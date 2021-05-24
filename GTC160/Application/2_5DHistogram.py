FRAME_WIDTH=160
FRAME_HEIGHT=120
FRAME_DEPTH=256



import PySimpleGUI as sg
import numpy as np
import cv2
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg,FigureCanvasAgg
from matplotlib.figure import Figure
from matplotlib import cm


def draw_figure(canvas,figure):
  figure_canvas_agg=FigureCanvasTkAgg(figure,canvas)
  figure_canvas_agg.draw()
  figure_canvas_agg.get_tk_widget().pack(side='top',fill='both',expand=1)
  return(figure_canvas_agg)

def main():
  layout=[
    [
      sg.Image(size=(640,480),key='_IMAGE_'),
      sg.Canvas(size=(640,480),key='_CANVAS_')
    ],
    [
      sg.Button('Exit',size=(10,1),pad=((280,0),3),font='Helvetica 14')
    ]
  ]
  window=sg.Window('2.5D Histrogram',layout,finalize=True)

  canvas=window['_CANVAS_'].TKCanvas


  camera=cv2.VideoCapture(0)
  # initial to YUV grab mode
  camera.set(cv2.CAP_PROP_CONVERT_RGB,0)
  camera.set(cv2.CAP_PROP_MODE,3)
  # initial camera shape
  camera.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_WIDTH)
  camera.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT)


  fig=Figure()
  ax=fig.add_subplot(111,projection='3d')
  ax.set_xlim3d([0,FRAME_WIDTH-1])
  ax.set_xlabel('X')
  ax.set_ylim3d([0,FRAME_HEIGHT-1])
  ax.set_ylabel('Y')
  ax.set_zlim3d([0,FRAME_DEPTH-1])
  ax.set_zlabel('Z')
  fig_agg=draw_figure(canvas,fig)


  data_x=np.zeros(shape=19200,dtype=np.uint8)
  data_y=np.zeros(shape=19200,dtype=np.uint8)
  data_z=np.zeros(shape=19200,dtype=np.uint8)

  cnt=0
  for y in range(FRAME_HEIGHT):
    for x in range(FRAME_WIDTH):
      data_x[cnt]=x
      data_y[cnt]=y
      cnt+=1

  while True:
    event,values=window.read(timeout=20)

    if event in ('Exit',None):
      exit(69)

    ret,img=camera.read()
    rawImg=np.reshape(img,newshape=((FRAME_HEIGHT*3)//2,FRAME_WIDTH))[:120][:]

    cnt=0
    for y in range(FRAME_HEIGHT):
      for x in range(FRAME_WIDTH):
        data_z[cnt]=rawImg[y][x]
        cnt+=1

    data_z[0]=255

    # clear the subplot
    ax.cla()
    # draw the grid
    ax.grid()
    ax.scatter(
      xs=data_x,
      ys=data_y,
      zs=data_z,
      s=1,
      c=data_z
    )
    fig_agg.draw()

    window['_IMAGE_'].update(
      data=cv2.imencode(
        ext='.png',
        img=cv2.resize(
          src=rawImg,
          dsize=(640,480),
          interpolation=cv2.INTER_LINEAR
        )
      )[1].tobytes()
    )

  window.close()
  camera.release()

if __name__ == '__main__':
    main()