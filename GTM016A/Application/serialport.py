########################################
# Title
#   module serialport
# Version
#   1.0
# Date
#   16 Sep, 2020
# Author
#   Riviere
# Descript
#   This module is designed to
#   handle serial port read and write
#   provide functions:
#     list_ports()
#     serial_open()
#     Serial_close()
#     send_data()
#     get_data()
#     get_array()
#     get_image()
########################################

import sys
import glob
import serial
from time  import sleep
from numpy import zeros
from numpy import uint8


WAIT_TIME=0.01


''' Lists out all usable serial port names
    raises EnvironmentError:
      On unsupported or unknown platforms
    returns:
      A list of the serial ports available on the system
'''
def list_ports(
):
  if sys.platform.startswith('win'):
    ports=['COM%s'%(i+1) for i in range(256)]
  elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
    # this excludes your current terminal "/dev/tty"
    ports=glob.glob('/dev/tty[A-Za-z]*')
  elif sys.platform.startswith('darwin'):
    ports=glob.glob('/dev/tty.*')
  else:
    raise EnvironmentError('Unsupported platform')

  result=[]
  for port in ports:
    try:
      s=serial.Serial(port)
      s.close()
      result.append(port)
    except(OSError,serial.SerialException):
      pass
  return(result)

''' connect port and returns port handler
'''
def serial_open(
  port,
  baudrate=115200,
  bytesize=serial.EIGHTBITS,
  parity=serial.PARITY_NONE,
  stopbits=serial.STOPBITS_ONE,
  timeout=10
):
  if port=='':
    return(None)
  try:
    port=serial.Serial(
      port=port,
      baudrate=baudrate,
      bytesize=bytesize,
      parity=parity,
      stopbits=stopbits,
      timeout=timeout
    )
    return(port)
  except(serial.SerialException):
    return(None)

''' disconnect port
'''
def serial_close(
  port
):
  port.close()

''' send bytes array to device
    host => device command+dataArray (4 bytes maximum)
    device => host ack (1 byte)
      TIMEOUT   : wait more than WAIT_TIME without ack
      DISCONNECT: SerialException
'''
def send_data(
  port,
  data,
  ack_check=None
):
  try:
    port.flushInput()
    port.write(data)
    sleep(WAIT_TIME)
    if port.in_waiting>0:
      # get one ack byte for scure check
      ack=int.from_bytes(port.read(1),byteorder='little')
      if ack_check!=None and ack_check!=ack:
        return('INCORRECT')
      else:
        return(ack)
    else:
      return('TIMEOUT')
  except(serial.SerialException):
    port.close()
    return('DISCONNECT')
  except(serial.portNotOpenError):
    return('DISCONNECT')

''' send bytes array to device and get ACK byte and data byte
    host => device command+dataArray (4 bytes maximum)
    device => host ack+data (2 byte)
      TIMEOUT   : wait more than WAIT_TIME without ack
      DISCONNECT: SerialException
'''
def get_data(
  port,
  data,
  ack_check=None
):
  try:
    port.flushInput()
    port.write(data)
    sleep(WAIT_TIME)
    if port.in_waiting>1:
      # get one ack byte for scure check
      ack=int.from_bytes(port.read(1),byteorder='little')
      if ack_check!=None and ack_check!=ack:
        return('INCORRECT')
      else:
        return(int.from_bytes(port.read(1),byteorder='little'))
    else:
      return('TIMEOUT')
  except(serial.SerialException):
    port.close()
    return('DISCONNECT')
  except(serial.portNotOpenError):
    return('DISCONNECT')

''' get data array
    device => host dataArray (length bytes)
      TIMEOUT   : wait more than wait_time without enough datas in waiting
      DISCONNECT: SerialException
'''
def get_array(
  port,
  length,
  wait_time=WAIT_TIME
):
  data=zeros(shape=(length),dtype=uint8)
  sleep(wait_time)
  try:
    if port.in_waiting>=length:
      for index in range(length):
        data[index]=int.from_bytes(port.read(1),byteorder='little')
      return(data)
    else:
      return('TIMEOUT')
  except serial.SerialException:
    port.close()
    return('DISCONNECT')
  except(serial.portNotOpenError):
    return('DISCONNECT')

''' get image
    device => host imageArray (length*2 bytes)
      descript:
        without handshake and scure check
        read 2 bytes into integer pixel in little endian mode
      TIMEOUT   : wait more than wait_time without enough datas in waiting
      DISCONNECT: SerialException
'''
def get_image(
  port,
  command,
  length,
  wait_time=WAIT_TIME
):
  data=zeros(shape=(length),dtype=uint8)
  try:
    port.flushInput()
    port.write([command])
    sleep(wait_time)
    if port.in_waiting>=length:
      for index in range(length):
        data[index]=int.from_bytes(port.read(2),byteorder='little')%256
      return(data)
    else:
      return('TIMEOUT')
  except(serial.SerialException):
    port.close()
    return('DISCONNECT')
  except(serial.portNotOpenError):
    return('DISCONNECT')

# ---------------------------------------
if __name__ == '__main__':
  # list_ports() test
  # print(list_ports())

  # serial_open() test
  port=serial_open('COM7')
  # print(port)

  # send_data() test: ack, TIMEOUT and DISCONNECT
  # for i in [33,33,34,34,35,35]:
  #   print(send_data(port,[33,9,0],i))
  #   print(port.isOpen())
  #   sleep(1)

  # get_data() test: ack, TIMEOUT and DISCONNECT
  # for i in [97,97,98,98,99,99]:
  #   print(get_data(port,[97,18],i))
  #   print(port.isOpen())
  #   sleep(1)

  # get_array() test: ack, TIMEOUT and DISCONNECT
  # print(send_data(port,[17]))
  # port.write([17])
  # sleep(0.02)
  # print(get_image(port,17,484,0.02))

  # serial_close() test:
  serial_close(port)
  # print(port)
  # del port