########################################
# Title
#   GTM016A
# Version
#   1.0
# Date
#   26 Aug, 2020
# Author
#   Riviere
# Descript
#   This App is designed to check fpga
########################################

import serialport as sp
import imageBuilder as ib
from time import sleep
import PySimpleGUI as sg
import numpy as np
import cv2
import os
import serial

########################################
# Global varables
########################################
# static variables
WINDOW_TITLE='GTM016A'
SCREEN_SIZE=[440,440]

# variable
portList=sp.list_ports()
portOpened=False
set_value=np.zeros(shape=(100),dtype=np.uint8)
get_value=np.zeros(shape=(100),dtype=np.uint8)
# frame=np.zeros(shape=(484),dtype=np.uint8)
calibration=np.zeros(shape=(1,18),dtype=np.uint8)
frame_size=[22,22]
frame_addr=[0,0,21,21]
decode='Dec'
grid=False
defaultport=portList[0]


# control panel generator
def controller(address,title,bit,attribute):
  if attribute=='R':
    attr=True
    attr_text='----'
  else:
    attr=False
    attr_text=''

  return([
    sg.Text(text='0',size=(4,1),justification='center'),
    sg.Text(text=str(address%256),size=(6,1),justification='center'),
    sg.Text(text=str(title),size=(20,1),justification='center'),
    sg.Text(text=str(bit),size=(5,1),justification='center'),
    sg.Text(text=str(attribute),size=(6,1),justification='center'),
    sg.Text(text="0",size=(7,1),key='get_'+str(address),justification='center'),
    sg.InputText(default_text=attr_text,size=(7,1),disabled=attr,key='set_'+str(address),justification='center'),
    sg.Button(button_text='Read',key='read_'+str(address),size=(5,1)),
    sg.Button(button_text='Write',disabled=attr,key='write_'+str(address),size=(5,1))
  ])

def checkBox(address,offset,title,bit,attribute):
  return([
    sg.Text(text='0',size=(4,1),justification='center'),
    sg.Text(text=str(address),size=(6,1),justification='center'),
    sg.Text(text=str(title),size=(20,1),justification='center'),
    sg.Text(text=str(bit),size=(5,1),justification='center'),
    sg.Text(text=str(attribute),size=(6,1),justification='center'),
    sg.Text(text='0',size=(7,1),key='get_'+str(address)+'_'+str(offset),justification='center'),
    sg.Checkbox(text='',size=(3,1),key='set_'+str(address)+'_'+str(offset)),
    sg.Button(button_text='Read',key='read_'+str(address)+'_'+str(offset),size=(5,1)),
    sg.Button(button_text='Write',key='write_'+str(address)+'_'+str(offset),size=(5,1))
  ])

# main window layout
WindowLayout=[
  [
    sg.Text('Serial port',(10,1)),
    sg.Combo(values=portList,size=(10,1),default_value=defaultport,key='__PORT__'),
    sg.Button('Connect',size=(10,1),key='__CONNECT__')
  ],
  [
    sg.Frame(title='',layout=[
      [sg.Image(filename='',background_color='black',key='__IMAGE__')],
      [
        sg.Text('',size=(20,1)),
        sg.Button('Grid Off',size=(10,1),key='__GRID__')
      ]
    ],border_width=0),
    sg.Frame(title='Registers',layout=[
      [
        sg.Text(text='Bank',size=(4,1),justification='center'),
        sg.Text(text='Address',size=(6,1),justification='center'),
        sg.Text(text='Name',size=(20,1),justification='center'),
        sg.Text(text='Bit',size=(5,1),justification='center'),
        sg.Text(text='Attribute',size=(6,1),justification='center'),
        sg.Text(text='Value',size=(7,1),justification='center'),
        sg.Combo(values=['Dec','Hex'],default_value=['Dec'],size=(5,1),key='__HEX__'),
        sg.Button(button_text='R all',key='read_all',size=(5,1)),
        sg.Button(button_text='W all',key='write_all',size=(5,1))
      ],
      controller(0,'PID[7:0]','[7:0]','R'),
      controller(1,'VID[7:0]','[7:0]','R'),
      controller(2,'slave_id[6:0]','[6:0]','R'),
      controller(3,'Reg_regreset','[0]','R/W'),
      controller(9,'wp_code[7:0]','[7:0]','R/W'),
      controller(18,'Reg_DigNp[7:0]','[7:0]','R/W'),
      controller(19,'Reg_DigNp[11:8]','[3:0]','R/W'),
      controller(20,'Reg_SysNp[7:0]','[7:0]','R/W'),
      controller(21,'Reg_SPI_MsNp[7:0]','[7:0]','R/W'),
      controller(281,'Reg_PCLKMask_enh','[2]','R/W'),
      controller(537,'Reg_SPICSMode','[3]','R/W'),
      controller(28,'Reg_Hstart_PreSync[4:0]','[4:0]','R/W'),
      controller(30,'Reg_Vstart_PreSync[4:0]','[4:0]','R/W'),
      controller(32,'Reg_Hend_PreSync[4:0]','[4:0]','R/W'),
      controller(34,'Reg_Vend_PreSync[4:0]','[4:0]','R/W'),
      controller(35,'Reg_CRstart_PreSync[4:0]','[4:0]','R/W'),
      controller(36,'Reg_CRend_PreSync[4:0]','[4:0]','R/W'),
      controller(38,'Reg_SYNC_length[4:0]','[4:0]','R/W'),
      controller(40,'Reg_PD[7:0]','[6]','R/W'),
      controller(90,'Reg_T_ANA_EN[7:0]','[6]','R/W'),
      controller(91,'Reg_T_EXT_VREF_sel[1:0]','[1:0]','R/W'),
      controller(93,'Reg_T_ipix_s[5:0]','[5:0]','R/W'),
      controller(94,'Reg_T_ADC_VREF_S','[4]','R/W'),
      controller(96,'Reg_T_PGA_GAIN[2:0]','[2:0]','R/W'),
    ])
  ]
]


########################################
# Sub function
########################################
def portDisconnect():
  window.Element('__CONNECT__').update('Connect')
  window.Element('__PORT__').update(disabled=False)

def getFrameSize():  # only read from stm
  tmp=sp.send_data(port,[11])
  if tmp=='TIMEOUT':
    sp.serial_close(port)
    portOpened=False
    portDisconnect()
    sg.popup('I2C NACK')
  elif tmp=='DISCONNECT':
    portOpened=False
    portDisconnect()
  else:
    tmp=sp.get_array(port,3,0.05)
    if not type(tmp)==str:
      frame_size[0]=int(tmp[0])
      frame_addr[1]=int(tmp[1])
      frame_addr[3]=int(tmp[2])
  tmp=sp.send_data(port,[10])
  if tmp=='TIMEOUT':
    sp.serial_close(port)
    portOpened=False
    portDisconnect()
    sg.popup('I2C NACK')
  elif tmp=='DISCONNECT':
    portOpened=False
    portDisconnect()
  else:
    tmp=sp.get_array(port,3,0.05)
    if not type(tmp)==str:
      frame_size[1]=int(tmp[0])
      frame_addr[0]=int(tmp[1])
      frame_addr[2]=int(tmp[2])

def drawGraph(screen,zoom_ratio,frame,frame_size,calibration,grid):
  graph=ib.imageShift(
    destination_size=(22,22),
    data=np.reshape(frame,frame_size),
    point1=(frame_addr[0],frame_addr[1]),
    point2=(frame_addr[2]+1,frame_addr[3]+1),
    border=grid,
    zoom_ratio=20
  )

  # nullpxl=np.zeros(shape=(1,2),dtype=np.uint8)
  # frame2=np.hstack([nullpxl,calibration,nullpxl])
  # frame1=cv2.resize(frame,(frame.shape[1]*zoom_ratio,frame.shape[0]*zoom_ratio),interpolation=cv2.INTER_NEAREST)
  # frame2=cv2.resize(frame2,(frame2.shape[1]*zoom_ratio,frame2.shape[0]*zoom_ratio),interpolation=cv2.INTER_NEAREST)

  # if grid:
  #   cv2.rectangle(img=frame1,pt1=(0,0),pt2=(frame1.shape[1]-1,frame1.shape[0]-1),color=(255))
  #   cv2.rectangle(img=frame2,pt1=(2*zoom_ratio,0),pt2=(frame2.shape[1]-2*zoom_ratio,frame2.shape[0]-1),color=(255))
  #   for x in range(frame1.shape[0]):
  #     if x%zoom_ratio==0 and x>0:
  #       cv2.line(img=frame1,pt1=(0,x),pt2=(frame1.shape[0]-1,x),color=(255))
  #       cv2.line(img=frame1,pt1=(x,0),pt2=(x,frame1.shape[0]-1),color=(255))
  #   for x in range(frame2.shape[1]):
  #     if x%zoom_ratio==0 and 2*zoom_ratio<x and x<20*zoom_ratio:
  #       cv2.line(img=frame2,pt1=(x,0),pt2=(x,frame2.shape[0]-1),color=(255))
  # nullpxl=np.zeros(shape=(frame_addr[0],frame1.shape[1]),dtype=np.uint8)
  # graph=np.vstack([nullpxl,frame1])
  # nullpxl=np.zeros(shape=(graph.shape[0],frame_addr[1]),dtype=np.uint8)
  # graph=np.hstack([nullpxl,graph])
  # nullpxl=np.zeros(shape=((SCREEN_SIZE[0]-graph.shape[0],graph.shape[1])),dtype=np.uint8)
  # graph=np.vstack([graph,nullpxl])
  # nullpxl=np.zeros(shape=((graph.shape[0],SCREEN_SIZE[1]-graph.shape[1])),dtype=np.uint8)
  # graph=np.hstack([graph,nullpxl])

  window[screen].update(
    data=cv2.imencode(ext='.png',img=graph)[1].tobytes()
  )

def convert(value,index,limit,decode):
  data=0
  if decode=='Dec':
    try:
      if int(value)>limit:
        window.Element('set_'+str(index)).update(str(limit))
      data=int(value)
    except ValueError:
      window.Element('set_'+str(index)).update('0')

  elif decode=='Hex':
    try:
      if int(value,16)>limit:
        window.Element('set_'+str(index)).update(str(hex(limit)))
      data=int(value,16)
    except ValueError:
      window.Element('set_'+str(index)).update('0x0')

  return(data)

def setValue(value,decode):
  data=np.zeros(shape=(256),dtype=np.uint8)

  data[3]=convert(value['set_3'],3,1,decode)
  data[9]=convert(value['set_9'],9,255,decode)
  data[18]=convert(value['set_18'],18,255,decode)
  data[19]=convert(value['set_19'],19,15,decode)
  data[20]=convert(value['set_20'],20,255,decode)
  data[21]=convert(value['set_21'],21,255,decode)
  # data[25]=convert(value['set_3'],25,12,decode)
  data[28]=convert(value['set_28'],28,31,decode)
  data[30]=convert(value['set_30'],30,31,decode)
  data[32]=convert(value['set_32'],32,31,decode)
  data[34]=convert(value['set_34'],34,31,decode)
  data[35]=convert(value['set_35'],35,31,decode)
  data[36]=convert(value['set_36'],36,31,decode)
  data[38]=convert(value['set_38'],38,31,decode)
  # data[40]=convert(value['set_40'],40,64,decode)
  # data[90]=convert(value['set_90'],90,64,decode)
  data[91]=convert(value['set_91'],91,3,decode)
  data[93]=convert(value['set_93'],93,63,decode)
  # data[94]=convert(value['set_94'],94,16,decode)
  data[96]=convert(value['set_96'],96,7,decode)

  # 281
  tmp=0
  try:
    if int(value['set_281'])!=0:
      window.Element('set_281').update('1')
      tmp+=4
  except ValueError:
    window.Element('set_281').update('0')
  # 537
  try:
    if int(value['set_537'])!=0:
      window.Element('set_537').update('1')
      tmp+=8
  except ValueError:
    window.Element('set_537').update('0')
  
  data[25]=tmp
  # 40
  try:
    if int(value['set_40'])!=0:
      window.Element('set_40').update('1')
      data[40]=64
    else:
      data[40]=0
  except ValueError:
    window.Element('set_40').update('0')
  # 90
  try:
    if int(value['set_90'])!=0:
      window.Element('set_90').update('1')
      data[90]=64
    else:
      data[90]=0
  except ValueError:
    window.Element('set_90').update('0')
  # 94
  try:
    if int(value['set_94'])!=0:
      window.Element('set_94').update('1')
      data[94]=16
    else:
      data[94]=0
  except ValueError:
    window.Element('set_94').update('0')

  # for index in range(len(keys)):
  #   if decode=='Dec' and type(list(keys)[index])==str and list(values)[index]!='----' and list(keys)[index][:4]=='set_':
  #     try:
  #       # print(int(list(keys)[index].split('_')[1])%256)
  #       # if int(list(keys)[index].split('_')[1])%256==25:
  #       #   data[25]=int(list(values)[index])
  #       data[int(list(keys)[index].split('_')[1])%256]=int(list(values)[index])
  #     except ValueError:
  #       window.Element(list(keys)[index]).update('0')

  return(data)

def getValue(value,decode):
  if decode=='Dec':
    window.Element('get_0').update(value[0])
    window.Element('get_1').update(value[1])
    window.Element('get_2').update(value[2]&127)
    window.Element('get_3').update(int((value[3]&1)==1))
    window.Element('get_9').update(value[9])
    window.Element('get_18').update(value[18])
    window.Element('get_19').update(value[19])
    window.Element('get_20').update(value[20])
    window.Element('get_21').update(value[21])
    window.Element('get_281').update(int((value[25]&4)==4))
    window.Element('get_537').update(int((value[25]&8)==8))
    window.Element('get_28').update(value[28])
    window.Element('get_30').update(value[30])
    window.Element('get_32').update(value[32])
    window.Element('get_34').update(value[34])
    window.Element('get_35').update(value[35])
    window.Element('get_36').update(value[36])
    window.Element('get_38').update(value[38])
    window.Element('get_40').update(value[40])
    window.Element('get_90').update(value[90] and 64)
    window.Element('get_91').update(value[91] and 3)
    window.Element('get_93').update(value[93] and 63)
    window.Element('get_94').update(value[94] and 16)
    window.Element('get_96').update(value[96] and 7)
  elif decode=='Hex':
    window.Element('get_0').update(hex(value[0]))
    window.Element('get_1').update(hex(value[1]))
    window.Element('get_2').update(hex(value[2]))
    window.Element('get_3').update(hex(value[3]))
    window.Element('get_9').update(hex(value[9]))
    window.Element('get_18').update(hex(value[18]))
    window.Element('get_19').update(hex(value[19]))
    window.Element('get_20').update(hex(value[20]))
    window.Element('get_21').update(hex(value[21]))
    window.Element('get_281').update(hex(int((value[25] & 4)==4)))
    window.Element('get_537').update(hex(int((value[25] & 8)==8)))
    window.Element('get_28').update(hex(value[28]))
    window.Element('get_30').update(hex(value[30]))
    window.Element('get_32').update(hex(value[32]))
    window.Element('get_34').update(hex(value[34]))
    window.Element('get_35').update(hex(value[35]))
    window.Element('get_36').update(hex(value[36]))
    window.Element('get_38').update(hex(value[38]))
    window.Element('get_40').update(hex(value[40]))
    window.Element('get_90').update(hex(value[90]))
    window.Element('get_91').update(hex(value[91]))
    window.Element('get_93').update(hex(value[93]))
    window.Element('get_94').update(hex(value[94]))
    window.Element('get_96').update(hex(value[96]))

def testFramed(light):
  frame=np.zeros(shape=(22,22),dtype=np.uint8)
  # test frame type A
  # for i in range(11):
  #   for j in range(11):
  #     frame[i*2][j*2]=light
  # test frame type B
  for i in range(22):
    for j in range(22):
      frame[i][j]=i*j*255//484
  return(frame)

def testCalibration(light):
  frame=np.zeros(shape=(1,18),dtype=np.uint8)
  for i in range(frame.shape[1]):
    frame[0][i]=light*i//18
  return(frame)

########################################
# Main initial function
########################################
window=sg.Window(WINDOW_TITLE).Layout(WindowLayout)

########################################
# Loop function
########################################
while True:
  event,values=window.read(timeout=200)

  # close this app
  if event==sg.WIN_CLOSED:
    break

  # refresh port list
  if not portOpened:
    portList=sp.list_ports()
    window.Element('__PORT__').update(values=portList)

  # serial click events
  if event=='__CONNECT__':
    if not portOpened:
      port=sp.serial_open(values['__PORT__'])
      if port!=None:
        portOpened=True
        window.Element('__CONNECT__').update('Disconnect')
        window.Element('__PORT__').update(disabled=True)
    else:
      portOpened=False
      sp.serial_close(port)
      portDisconnect()

  # image grid switch
  if event=='__GRID__':
    if grid:
      grid=False
      window.Element('__GRID__').update('Grid Off')
    else:
      grid=True
      window.Element('__GRID__').update('Grid On')

  # read event
  if event[:5]=='read_' and portOpened:
    if event[5:]=='all':
    # for loop read
      for addr in [0,1,2,3,9,18,19,20,21,25,28,30,32,34,35,36,38,40]:
        tmp=sp.get_data(port,[97,addr])
        if tmp=='TIMEOUT':
          sp.serial_close(port)
          portOpened=False
          portDisconnect()
          sg.popup('I2C NACK')
          break
        elif tmp=='DISCONNECT':
          portOpened=False
          portDisconnect()
          break
        else:
          get_value[addr]=tmp
    # continues read
      # tmp=sp.send_data(port,[100])
      # if tmp=='TIMEOUT':
      #   sp.serial_close(port)
      #   portOpened=False
      #   portDisconnect()
      #   sg.popup('I2C NACK')
      # elif tmp=='DISCONNECT':
      #   portOpened=False
      #   portDisconnect()
      # else:
      #   tmp=sp.get_array(port,45,0.1)
      #   if not type(tmp)==str:
      #     for index in range(45):
      #       get_value[index]=tmp[index]
    else:
      addr=int(event[5:])%256
      tmp=sp.get_data(port,[97,addr])
      if tmp=='TIMEOUT':
        sp.serial_close(port)
        portOpened=False
        portDisconnect()
        sg.popup('I2C NACK')
      elif tmp=='DISCONNECT':
        portOpened=False
        portDisconnect()
      else:
        get_value[addr]=tmp

  # write event
  if event[:6]=='write_' and portOpened:
    if event[6:]=='all':
      for addr in [3,9,18,19,20,21,25,28,30,32,34,35,36,38,40]:
        tmp=sp.send_data(port,[33,addr,set_value[addr]])
        if tmp=='TIMEOUT':
          port.close()
          portOpened=False
          portDisconnect()
          sg.popup('I2C NACK')
          break
        if tmp=='DISCONNECT':
          portOpened=False
          portDisconnect()
    else:
      addr=int(event[6:])%256
      tmp=sp.send_data(port,[33,addr,set_value[addr]])
      if tmp=='TIMEOUT':
        port.close()
        portOpened=False
        portDisconnect()
        sg.popup('I2C NACK')
      if tmp=='DISCONNECT':
        portOpened=False
        portDisconnect()
    getFrameSize()

  # get and draw image
  if portOpened:
    getFrameSize()
    frame=np.zeros(shape=(frame_size[0]*frame_size[1]),dtype=np.uint8)
    drawGraph(screen='__IMAGE__',zoom_ratio=20,frame=frame,frame_size=frame_size,calibration=calibration,grid=grid)
    tmp=sp.get_image(port,17,frame_size[0]*frame_size[1],0.02)
    if type(tmp)==str and tmp=='DISCONNECT':
      portOpened=False
      portDisconnect()
      sg.popup('PORT DISCONNECTED')
    if not type(tmp)==str:
      frame=tmp

    # print(frame)
    # port.write([20])
    # sleep(0.02)
    # # print(calibration.size)
    # if port.in_waiting>0:
    #   for index in range(calibration.size):
    #     calibration[0][index]=int.from_bytes(port.read(2),byteorder='little')%256
    # port.flushInput()
    # # print(calibration)
    drawGraph(screen='__IMAGE__',zoom_ratio=20,frame=frame,frame_size=frame_size,calibration=calibration,grid=grid)
  else:
    frame_size=[22,22]
    drawGraph(screen='__IMAGE__',zoom_ratio=20,frame=ib.testFrame(255),frame_size=frame_size,calibration=testCalibration(255),grid=grid)

  # print('running...')

  # sync gui label with set and get caches
  set_value=setValue(values,values['__HEX__'])
  getValue(get_value,values['__HEX__'])


########################################
# Memery recycle
########################################
if portOpened:
  port.close()
window.close()
