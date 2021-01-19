FRAME_WIDTH=160
FRAME_HEIGHT=120
FRAME_DEPTH=256


import numpy as np
import matplotlib.pyplot as plt
import mpl_toolkits.mplot3d.axes3d as p3
import matplotlib.animation as animation
import cv2

# def Gen_RandLine(length, dims=2):
#     """
#     Create a line using a random walk algorithm

#     length is the number of points for the line.
#     dims is the number of dimensions the line has.
#     """
#     lineData = np.empty((dims, length))
#     lineData[:, 0] = np.random.rand(dims)
#     for index in range(1, length):
#         # scaling the random numbers by 0.1 so
#         # movement is small compared to position.
#         # subtraction by 0.5 is to change the range to [-0.5, 0.5]
#         # to allow a line to move backwards.
#         step = ((np.random.rand(dims) - 0.5) *100)
#         lineData[:, index] = lineData[:, index - 1] + step

#     return lineData


# def update_lines(num, dataLines, lines):
#     for line, data in zip(lines, dataLines):
#         # NOTE: there is no .set_data() for 3 dim data...
#         line.set_data(data[0:2, :num])
#         line.set_3d_properties(data[2, :num])
#     return lines


def update_data(d):
  ret,img=camera.read()
  rawImg=np.reshape(img,newshape=((FRAME_HEIGHT*3)//2,FRAME_WIDTH))[:120][:]

  cnt=0
  for y in range(FRAME_HEIGHT):
    for x in range(FRAME_WIDTH):
      data_z[cnt]=rawImg[y][x]
      cnt+=1

  data_z[0]=255

  ax.cla()
  ax.scatter(data_x,data_y,data_z,s=1,c=data_z)


# Attaching 3D axis to the figure
fig = plt.figure()
ax = p3.Axes3D(fig)

# Setting the axes properties
ax.set_title('3D Histrogram')
ax.set_xlim3d([0, 159])
ax.set_xlabel('X')
ax.set_ylim3d([119,0])
ax.set_ylabel('Y')
ax.set_zlim3d([0, 255])
ax.set_zlabel('Value')


camera=cv2.VideoCapture(0)
# initial to YUV grab mode
camera.set(cv2.CAP_PROP_CONVERT_RGB,0)
camera.set(cv2.CAP_PROP_MODE,3)
# initial camera shape
camera.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_WIDTH)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT)


data_x=np.zeros(shape=19200,dtype=np.uint8)
data_y=np.zeros(shape=19200,dtype=np.uint8)
data_z=np.zeros(shape=19200,dtype=np.uint8)
cnt=0
for y in range(FRAME_HEIGHT):
  for x in range(FRAME_WIDTH):
    data_x[cnt]=x
    data_y[cnt]=y
    cnt+=1



# Fifty lines of random 3-D lines
# data = [Gen_RandLine(25, 3) for index in range(50)]
# Creating fifty line objects.
# NOTE: Can't pass empty arrays into 3d version of plot()
# lines = [ax.scatter(dat[0, 0:100], dat[1, 0:100], dat[2, 0:100]) for dat in data]


# Creating the Animation object
line_ani = animation.FuncAnimation(fig, update_data, interval=50, blit=False)

plt.show()
