import os
from time import sleep
import json
import numpy as np
import PySimpleGUI as sg

########################################
# Var
########################################
run_status=False
cid_selected=''
chip_id=''
cpid_changed=0
data={'database':[]}

layout_path=[
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
    sg.ProgressBar(max_value=100,size=(33, 25),key='__PBAR__'),
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
  ]
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

def start_testing():
  tmp={'current':0,'PCLK':0,'dark_std':0,'reference_std':0,'active_std':0}
  for i in range(101):
    window['__PBAR__'].UpdateBar(i)
    sleep(0.01)
  tmp['current']=10.5
  tmp['PCLK']=66.6
  tmp['dark_std']=12.7
  tmp['reference_std']=18.5
  tmp['active_std']=24.1
  return(tmp)

def result_check(result):
          # if( 10<tmp['current']       and tmp['current']<100      and
          #     10<tmp['PCLK']          and tmp['PCLK']<300         and
          #     10<tmp['dark_std']      and tmp['dark_std']<20      and
          #     10<tmp['reference_std'] and tmp['reference_std']<20 and
          #     10<tmp['active_std']    and tmp['active_std']<20    ):
  return('FAIL',{'current','PCLK'})
########################################
# Init
########################################
refreshPathList('INIT')

########################################
# Main loop
########################################
while True:
  event,values=window.read(timeout=200)
  if event==sg.WIN_CLOSED:
    break
  if values['__LIST__']:
    path=os.getcwd()+'\\'+values['__LIST__'][0]
    window['__PATH__'].update(path)
    refreshPathList(path)
  if event=='__RUN__':
    run_status=True
    window['__PATH__'].update(disabled=True,text_color='grey')
    window['__LIST__'].update(disabled=True)
    window['__RUN__'].update(disabled=True)

  if event=='__ADCP__' and values['__CPID__']!='':
    tmp=[p['id'] for p in data['database'] if values['__CPID__']==p['id']]
    if tmp!=[]:
      sg.popup('CID: '+values['__CPID__']+' already exist, use another Chip ID!',title='Caution')
    else:
      data['database'].append({'id':str(values['__CPID__']),'desc':'','current':0,'PCLK':0,'dark_std':0,'reference_std':0,'active_std':0,'result':''})
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
      cpid_changed=1
    cid_selected=values['__DATA__']

  if chip_id and cpid_changed:
    cpid_changed=0
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


  if event=='__TEST__':
    # save result into client only(won't push to database server)
    for p in data['database']:
      if p['id']==chip_id:
        p['desc']=values['__CPDE__']
        if 'OK'==sg.popup_ok_cancel('Check again Chip ID on socket!',title='Warning'):
          tmp=start_testing()
          sg.PopupQuick('Test complete!')

          p['current']=tmp['current']
          p['PCLK']=tmp['PCLK']
          p['dark_std']=tmp['dark_std']
          p['reference_std']=tmp['reference_std']
          p['active_std']=tmp['active_std']
          
          status,res=result_check(tmp)
          if status=='PASS':
            p['result']='PASS'
            window['__RESU__'].update(button_color=('black','#00ff00'))
          else:
            p['result']='FAIL'
            window['__RESU__'].update(button_color=('black','#ff0000'))
            # move to sub-function
            for p in res:
              print(p)
            #
          # window['__RESU__'].update(p['result'])
          window['__CRNT__'].update(tmp['current'])
          window['__PCLK__'].update(tmp['PCLK'])
          window['__DSTD__'].update(tmp['dark_std'])
          window['__RSTD__'].update(tmp['reference_std'])
          window['__ASTD__'].update(tmp['active_std'])
########################################
# Memery recycle
########################################
window.close()
