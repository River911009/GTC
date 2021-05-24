import os
from time import sleep
import json
import numpy as np
import PySimpleGUI as sg
import serialPort as sp

########################################
# Var
########################################
debugMode=True

port=sp.list_ports()
run_status=False
cid_selected=''
auto_selected=''
chip_id=''
cpid_changed=False
auto_add_cpid=False
data={'database':[]}

layout_path=[
  [sg.Text('GTM016AN',size=(10,1)),sg.DropDown(port,size=(17,1),key='__SENSOR__')],
  [sg.Text('Multimeter',size=(10,1)),sg.DropDown(port,size=(17,1),key='__METER__')],
  [
    sg.Text(text='Path:'),
    sg.Input(size=(24,1),key='__PATH__')
  ],
  [sg.Listbox([],size=(30,5),key='__LIST__')],
  [
    sg.Button('START',size=(24,1),key='__RUN__'),
    sg.Button('?',size=(2,1),key='__HELP__')
  ]
]

layout_data=[
  [sg.Listbox([],size=(30,15),key='__DATA__')],
  [
    sg.Text(text='CID:',size=(3,1)),
    sg.Input(default_text='',disabled=True,size=(13,1),key='__CPID__'),
    sg.Button(button_text='ADD',disabled=True,size=(4,1),key='__ADCP__'),
    sg.Button(button_text='DEL',disabled=True,size=(4,1),key='__DECP__')
  ],
  [
    sg.Button('READ',size=(13,1),key='__RD__'),
    sg.Button('WRITE',size=(13,1),disabled=True,key='__WR__')
  ]
]

layout_detail=[
  [
    sg.Text(text='Chip description:',size=(15,1)),
    sg.Multiline(default_text='',size=(38,4),key='__CPDE__')
  ],
  [
    sg.Text(text='Test result:',size=(15,1)),
    sg.Button(button_text='',size=(35,5),disabled=True,key='__RESU__')
  ],
  [
    sg.ProgressBar(max_value=1,size=(33, 25),key='__PBAR__'),
    sg.Button(button_text='TEST',size=(5,1),key='__TEST__')
  ],
  [sg.Text(text='',size=(10,1))],
  [sg.Text(text='Chip setting:',size=(15,1))],
  [
    sg.Text(text='FPS:',size=(15,1)),
    sg.Text(text='Test result:',size=(15,1))
  ],
  [
    sg.Text(text='IPIX:',size=(15,1)),
    sg.Text(text='Test result:',size=(15,1))
  ],
  [
    sg.Text(text='Vref:',size=(15,1)),
    sg.Text(text='Test result:',size=(15,1))
  ],
  [sg.Text('',size=(10,2))]
]

layout_value=[
  [
    sg.Text(text='current:',size=(15,1)),
    sg.Text(text='NULL',size=(10,1),key='__CRNT__'),
    sg.Text(text='',size=(23,1),text_color='red',key='__CRNTFAIL__')
  ],
  [
    sg.Text(text='PCLK:',size=(15,1)),
    sg.Text(text='NULL',size=(10,1),key='__PCLK__'),
    sg.Text(text='',size=(23,1),text_color='red',key='__PCLKFAIL__')
  ],
  [
    sg.Text(text='Dark STD:',size=(15,1)),
    sg.Text(text='NULL',size=(10,1),key='__DSTD__'),
    sg.Text(text='',size=(23,1),text_color='red',key='__DSTDFAIL__')
  ],
  [
    sg.Text(text='Reference STD:',size=(15,1)),
    sg.Text(text='NULL',size=(10,1),key='__RSTD__'),
    sg.Text(text='',size=(23,1),text_color='red',key='__RSTDFAIL__')
  ],
  [
    sg.Text(text='Active STD:',size=(15,1)),
    sg.Text(text='NULL',size=(10,1),key='__ASTD__'),
    sg.Text(text='',size=(23,1),text_color='red',key='__ASTDFAIL__')
  ],
  [sg.Text(text='',size=(10,1))]
]

layout_ui=[
  sg.Frame(title='',layout=[
      [sg.Frame(title='Path setting',layout=layout_path,border_width=1)],
      [sg.Frame(title='Data log',layout=layout_data,border_width=1)]
    ],border_width=0),
  sg.Frame(title='',layout=[
      [sg.Frame(title='Detail',layout=layout_detail,border_width=1)],
      [sg.Frame(title='Test Value',layout=layout_value,border_width=1)]
    ],border_width=0)
]

window=sg.Window(title="GTM016AN Chip Test Logger",layout=[layout_ui]).finalize()

########################################
# Sub func
########################################
def debug_database(database,key):
  print([p[key] for p in database['database']])

def refreshPathList(path):
  if path=='INIT':
    pset_list=[a for a in os.listdir('./') if not a.startswith('.')]
    window['__LIST__'].update(pset_list)
    window['__PATH__'].update(os.getcwd())
  elif os.getcwd()!=path:
    try:
      os.chdir(path)
      pset_list=[a for a in os.listdir('./') if not a.startswith('.')]
      pset_list.insert(0,'..')
      window['__LIST__'].update(pset_list)
    except:
      pass

def refreshDataList(database):
  tmp=[]
  for p in database['database']:
    tmp.append(p['id'])
  window['__DATA__'].update(tmp)

def start_testing(samples=100):
  tmp={'current':0,'PCLK':0,'dark_std':0,'reference_std':0,'active_std':0}

  meter=sp.serial_open(port=values['__METER__'])
  # measure chip current and convert to mA
  sp.send_data(meter,bytes("MEASure:CURRent:DC? \r\n",'ascii'))
  result=meter.readline()
  tmp['current']=(format(1000*float(result.decode('utf-8')),'f'))

  window['__PBAR__'].UpdateBar(0.1)

  # measure PCLK frequency and convert to kHz
  sp.send_data(meter,bytes("MEASure:FREQuency?\r\n",'ascii'))
  result=meter.readline()
  tmp['PCLK']=(format(0.031*float(result.decode('utf-8')),'f'))
  sp.send_data(meter,bytes("MEASure:FREQuency?\r\n",'ascii'))
  meter.close()

  window['__PBAR__'].UpdateBar(0.2)

  # start measure GTM016AN image
  gtm016an=sp.serial_open(port=values['__SENSOR__'])
  rawdata=np.zeros((samples,22),dtype=np.uint32)
  for loop in range(samples):
    gtm016an.flushInput()
    gtm016an.write([20])
    sleep(0.01)
    if gtm016an.in_waiting>=22:
      for index in range(22):
        rawdata[loop][index]=int.from_bytes(gtm016an.read(2),byteorder='little')

    window['__PBAR__'].UpdateBar(0.2+loop/samples)

  gtm016an.close()

  # start calculate pixel std
  stdData=np.std(rawdata,axis=0)
  tmp['dark_std']=(stdData[0]+stdData[21])/2
  tmp['reference_std']=(stdData[1]+stdData[20])/2
  tmp['active_std']=np.sum(stdData[2:20])/18
  if debugMode:
    debug_database(data,'current')
    debug_database(data,'PCLK')
    print(stdData)

  return(tmp)

def result_check(result):
  limit={ 'currentLT':  10,'currentGT': 100,
          'PCLKLT':     10,'PCLKGT':    300,
          'dark_stdLT': 10,'dark_stdGT':30,
          'ref_stdLT':  10,'ref_stdGT': 30,
          'act_stdLT':  10,'act_stdGT': 20}
  status='PASS'
  tmp={}

  if float(result['current'])<limit['currentLT']:
    tmp['__CRNTFAIL__']='<'+str(limit['currentLT'])
  elif float(result['current'])>limit['currentGT']:
    tmp['__CRNTFAIL__']='>'+str(limit['currentGT'])
  
  if float(result['PCLK'])<limit['PCLKLT']:
    tmp['__PCLKFAIL__']='<'+str(limit['PCLKLT'])
  elif float(result['PCLK'])>limit['PCLKGT']:
    tmp['__PCLKFAIL__']='>'+str(limit['PCLKGT'])
  
  if float(result['dark_std'])<limit['dark_stdLT']:
    tmp['__DSTDFAIL__']='<'+str(limit['dark_stdLT'])
  elif float(result['dark_std'])>limit['dark_stdGT']:
    tmp['__DSTDFAIL__']='>'+str(limit['dark_stdGT'])

  if float(result['reference_std'])<limit['ref_stdLT']:
    tmp['__RSTDFAIL__']='<'+str(limit['ref_stdLT'])
  elif float(result['reference_std'])>limit['ref_stdGT']:
    tmp['__RSTDFAIL__']='>'+str(limit['ref_stdGT'])

  if float(result['active_std'])<limit['act_stdLT']:
    tmp['__ASTDFAIL__']='<'+str(limit['act_stdLT'])
  elif float(result['active_std'])>limit['act_stdGT']:
    tmp['__ASTDFAIL__']='>'+str(limit['act_stdGT'])

  if tmp!={}:
    status='FAIL'

  return(status,tmp)

########################################
# Init
########################################
os.chdir(os.getcwd()+'/GTM016AN')

refreshPathList('INIT')

########################################
# Main loop
########################################
while True:
  event,values=window.read(timeout=200)
  if event==sg.WIN_CLOSED:
    break

  if event=='__HELP__':
    help_str ='GTM016AN testlogger\t'
    help_str+='(version 1.0)\n'
    help_str+='\n'
    help_str+='designed to test STD of dark/reference/active pixels,\n'
    help_str+='active current and pclk frequency automatically.\n'
    help_str+='\n'
    help_str+='Created and managed by Riviere\n'
    help_str+='on 21 May, 2021\n'
    sg.popup_ok(help_str,title='Help centre')

  if values['__LIST__']:
    path=os.getcwd()+'\\'+values['__LIST__'][0]
    window['__PATH__'].update(path)
    refreshPathList(path)
  if event=='__RUN__':
    if values['__SENSOR__'] and values['__METER__'] and values['__SENSOR__']!=values['__METER__']:
      run_status=True
      window['__PATH__'].update(disabled=True,text_color='grey')
      window['__LIST__'].update(disabled=True)
      window['__RUN__'].update(disabled=True)
    else:
      sg.popup('Serial port wrong.')

  if (event=='__ADCP__' and values['__CPID__']!='') or auto_add_cpid:
    auto_add_cpid=False
    tmp=[p['id'] for p in data['database'] if values['__CPID__']==p['id']]
    if tmp!=[]:
      sg.popup('CID: '+values['__CPID__']+' already exist, use another Chip ID!',title='Caution')
    else:
      data['database'].append({'id':str(values['__CPID__']),'desc':'','current':0,'PCLK':0,'dark_std':0,'reference_std':0,'active_std':0,'result':'NOT TEST'})
      refreshDataList(data)
  if event=='__DECP__' and values['__DATA__']:
    for p in data['database']:
      if p['id']==values['__DATA__'][0]:
        data['database'].remove(p)
      refreshDataList(data)
  if run_status and event=='__WR__':
    data['database'].sort(key=lambda p:p['id'])
    with open('database.json','w') as jsonData:
      json.dump(data,jsonData)
      jsonData.close()
  if run_status and event=='__RD__':
    if not os.path.exists('database.json'):
      sg.popup_error('Can not find "database.json" or wrong file in this folder. Please change another path!',title='Error!')
      run_status=False
      window['__PATH__'].update(disabled=False,text_color='black')
      window['__LIST__'].update(disabled=False)
      window['__RUN__'].update(disabled=False)
    else:
      with open('database.json','r') as jsonData:
        data=json.load(jsonData)
        jsonData.close()
        refreshDataList(data)
      window['__CPID__'].update(disabled=False)
      window['__ADCP__'].update(disabled=False)
      window['__DECP__'].update(disabled=False)
      window['__WR__'].update(disabled=False)

  if values['__DATA__']:
    if cid_selected!=values['__DATA__']:
      window['__CPID__'].update(values['__DATA__'][0])
      chip_id=values['__DATA__'][0]
      cpid_changed=True
    cid_selected=values['__DATA__']
  elif auto_selected:
    if cid_selected!=auto_selected:
      window['__CPID__'].update(auto_selected)
      chip_id=auto_selected
      cpid_changed=True
    cid_selected=auto_selected
    auto_selected=''

  if chip_id and cpid_changed:
    cpid_changed=False
    tmp=[p for p in data['database'] if p['id']==chip_id]
    if tmp!=[]:
      window['__PBAR__'].UpdateBar(0)
      window['__CPDE__'].update(tmp[0]['desc'])
      window['__RESU__'].update(tmp[0]['result'])
      if tmp[0]['result']=='FAIL':
        window['__RESU__'].update(button_color=('black','#ff0000'))
      elif tmp[0]['result']=='PASS':
        window['__RESU__'].update(button_color=('black','#00ff00'))
      else:
        window['__RESU__'].update(button_color=sg.theme_button_color())
      window['__CRNT__'].update(tmp[0]['current'])
      window['__PCLK__'].update(tmp[0]['PCLK'])
      window['__DSTD__'].update(tmp[0]['dark_std'])
      window['__RSTD__'].update(tmp[0]['reference_std'])
      window['__ASTD__'].update(tmp[0]['active_std'])

      status,res=result_check(tmp[0])

      window['__CRNTFAIL__'].update('')
      window['__PCLKFAIL__'].update('')
      window['__DSTDFAIL__'].update('')
      window['__RSTDFAIL__'].update('')
      window['__ASTDFAIL__'].update('')

      if tmp[0]['result']=='PASS':
        window['__RESU__'].update(button_color=('black','#00ff00'))
      elif tmp[0]['result']=='FAIL':
        window['__RESU__'].update(button_color=('black','#ff0000'))
        for f in res:
          window[f].update(res[f])

  if event=='__TEST__':
    # save result into client only(won't push to database server)
    for p in data['database']:
      # search one by one, should be only one state in 'if'
      if p['id']==chip_id:
        # update desc after found result
        p['desc']=values['__CPDE__']
        # double check before start testing
        if 'OK'==sg.popup_ok_cancel('Check again Chip ID on socket!',title='Warning'):
          tmp=start_testing()
          # sg.PopupQuick('Test complete!')
          # update result if test complete
          p['current']=tmp['current']
          p['PCLK']=tmp['PCLK']
          p['dark_std']=tmp['dark_std']
          p['reference_std']=tmp['reference_std']
          p['active_std']=tmp['active_std']

          window['__CRNT__'].update(tmp['current'])
          window['__PCLK__'].update(tmp['PCLK'])
          window['__DSTD__'].update(tmp['dark_std'])
          window['__RSTD__'].update(tmp['reference_std'])
          window['__ASTD__'].update(tmp['active_std'])

          status,res=result_check(tmp)

          window['__CRNTFAIL__'].update('')
          window['__PCLKFAIL__'].update('')
          window['__DSTDFAIL__'].update('')
          window['__RSTDFAIL__'].update('')
          window['__ASTDFAIL__'].update('')

          window['__RESU__'].update(status)
          if status=='PASS':
            p['result']='PASS'
            window['__RESU__'].update(button_color=('black','#00ff00'))
          else:
            p['result']='FAIL'
            window['__RESU__'].update(button_color=('black','#ff0000'))
            for f in res:
              window[f].update(res[f])

          if 'Yes'==sg.popup_yes_no('Test completed, like to add and test next chip?',title='Next step'):
            index=0
            for p in data['database']:
              if index<int(p['id']):
                index=int(p['id'])
            window['__CPID__'].update(str(index+1))
            auto_add_cpid=True
            auto_selected=str(index+1)

########################################
# Memery recycle
########################################
window.close()
