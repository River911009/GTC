import serialPort as sp
import os


os.chdir(os.path.dirname(os.path.abspath(__file__)))
print(os.getcwd())
portList=sp.list_ports()

GTM016=sp.serial_open(port=portList[1])
Meter=sp.serial_open(port=portList[2])


def measure(ipix=0,wait=1):
  sp.send_data(GTM016,[33,93,ipix])
  sp.sleep(wait)
  sp.send_data(Meter,bytes("READ?\r\n",'ascii'))
  result=Meter.readline()
  print(ipix,format(1000000*float(result.decode('utf-8')),'f'))
  return(format(1000000*float(result.decode('utf-8')),'f'))


print('Initial GTM016 and GDM-9061...')
sp.send_data(GTM016,[33,9,90])
sp.send_data(Meter,bytes("MEASure:CURRent:DC?\r\n",'ascii'))

if sp.get_data(GTM016,[97,9])==90:
  print('Great! start logging Ipix now...')
  with open('.\ipix_measured.csv','w') as f:
    f.write('Ipix[5:0], Iset, measured,\n')
    for ipix in range(64):
      f.write(str(ipix))
      f.write(', ')
      f.write(str(ipix+1))
      f.write(', ')
      f.write(measure(ipix=ipix,wait=1))
      f.write(',\n')
    f.close()
  print('Congrats! Data logged at ipix_measured.csv\nBye!')
else:
  print('Sorry! But something wrong...')

sp.send_data(GTM016,[33,9,0])
sp.serial_close(GTM016)
sp.serial_close(Meter)
