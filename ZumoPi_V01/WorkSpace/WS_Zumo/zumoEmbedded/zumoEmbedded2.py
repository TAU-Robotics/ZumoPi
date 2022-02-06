# Run this app with `python app.py` and
# visit http://192.168.0.102:8050/ in your web browser.

import dash
#import dash_html_components as html
from dash import html
#import dash_core_components as dcc
from dash import dcc
#import plotly.express as px
#import pandas as pd
import dash_daq as daq
import serial
import time, math
import threading
import json
import plotly.graph_objs as go
import plotly
import logging

X = [0]
Y = [0]
prox_sensor_names = ['Left-Left', 'Left', 'Center', 'Right', 'Right-Right']
line_sensor_values = [0, 0, 0, 0, 0]

clockwise_arr = [0] * 100
counter_clockwise_arr = [0] * 100

counter = 0

ser = serial.Serial('/dev/ttyACM0')  # open serial port
#ser = serial.Serial('COM3', baudrate = 9600, timeout = 1)

# variables
zumoAngle = 0
zumoSpeed = 0
auto_flag = 0

# Initialise the app
app = dash.Dash(__name__)

# Define the app
app.layout = html.Div(children=[daq.Joystick(id='my-joystick', label="Zumo Joystick", angle=0, size=100),
                                html.Div(id='joystick-output'),
                                html.Div(className='row', children=[html.Button('Automatic', id='auto_button', n_clicks=0),
                                                                    html.Button('Manual', id='manual_button', n_clicks=0),
                                                                    html.Div(id='Button output'),
                                                                    html.Div([dcc.Graph(id = 'live-graph', animate = True),
                                                                              dcc.Interval(id = 'graph-update', interval = 1000, n_intervals = 0),
                                                                    html.Div([dcc.Graph(id = 'live-barchart', animate = True),
                                                                              dcc.Interval(id = 'barchart-update', interval = 1000, n_intervals = 0)])
])])])

@app.callback(dash.dependencies.Output('live-graph', 'figure'),
              dash.dependencies.Input('graph-update', 'n_intervals'))

def update_graph_scatter(n):
    X.append(X[-1]+1)
    Y.append(Y[-1]+1)

    data = go.Scatter(x=list(X), y=list(Y), name='Scatter', mode='lines+markers')
    return {'data': [data],
            'layout': go.Layout(xaxis=dict(range=[min(X), max(X)]),
                                yaxis=dict(range=[min(Y), max(Y)]))}

@app.callback(dash.dependencies.Output('live-barchart', 'figure'),
              [dash.dependencies.Input('barchart-update', 'n_intervals')])

def update_graph_bar(n):
  clrs = []
  colorred = 'rgb(222,0,0)'
  colorgreen = 'rgb(0,222,0)'
  for i in range(len(line_sensor_values)):
    if (line_sensor_values[i] > 5):
      clrs.append(colorred)
    else:
      clrs.append(colorgreen)

  traces = list()
  traces.append(plotly.graph_objs.Bar(x=prox_sensor_names, y=line_sensor_values, name='Bar', marker=dict(color=clrs)))
  layout = go.Layout(barmode='group', yaxis=dict(range=[0, max(line_sensor_values)]))
  if traces is not None and layout is not None:
    return {'data': traces, 'layout': layout}

@app.callback(dash.dependencies.Output('Button output', 'children'),
              [dash.dependencies.Input('auto_button', 'n_clicks'),
              dash.dependencies.Input('manual_button', 'n_clicks')])

def displayClick(auto_button, manual_button):
  global auto_flag, clockwise_arr, counter_clockwise_arr
  msg = 'manual_button was most recently clicked'
  changed_id = [p['prop_id'] for p in dash.callback_context.triggered][0]
  if 'auto_button' in changed_id:
    msg = 'auto_button was most recently clicked'
    auto_flag = 1
    clockwise_arr = [0] * 100
    counter_clockwise_arr = [0] * 100
  elif 'manual_button' in changed_id:
    msg = 'manual_button was most recently clicked'
    auto_flag = 0

    return html.Div(msg)

@app.callback(
    dash.dependencies.Output('joystick-output', 'children'),
    [dash.dependencies.Input('my-joystick', 'angle'),
     dash.dependencies.Input('my-joystick', 'force')])

def update_output(angle, force=0):
    print("update_output")
    global zumoSpeed, zumoAngle
    if force is None:
        force = 0
    if type(angle) == int or float:
      zumoAngle = angle
    else:
      zumoAngle = 0

    if type(force) == int or float:
      zumoSpeed = force
    else:
      zumoSpeed = 0

    return ['Angle is {}'.format(angle),
            html.Br(),
            'Force is {}'.format(zumoSpeed)]

def TransmitThread():
  print ("transmitThread")
  global control_params, clockwise_arr, counter_clockwise_arr
  while ser.isOpen:
    global zumoSpeed, zumoAngle, auto_flag
    if zumoAngle is None:
      zumoAngle = 0
    if zumoSpeed is None:
      zumoSpeed = 0
    joyY = math.sin(math.radians(zumoAngle))*zumoSpeed*200
    joyX = math.cos(math.radians(zumoAngle))*zumoSpeed*200
    if sum(counter_clockwise_arr) > sum(clockwise_arr):
      direction = '0'
    else:
      direction = '1'
    msg = ''
    msg = str(auto_flag) + ';' + direction + '.' + str(joyX) + ',' +str(joyY) +'\r\n'
    ser.write(msg.encode('ascii'))
    time.sleep(0.1)

def ReceiveThread():
  global line_sensor_values, clockwise_arr, counter_clockwise_arr, counter
  while(ser.isOpen):
    if (ser.in_waiting > 0):
      sensor = ser.readline().decode('ascii')
      data = json.loads(sensor)
      #for key in data.keys():
        #print(key + ": " + str(data[key]))
      for i in range(len(data["line_sensor_values"])):
        line_sensor_values[i] = data["line_sensor_values"][i]
      counter_clockwise_arr[counter%100] = line_sensor_values[0] + line_sensor_values[1]
      clockwise_arr[counter%100] = line_sensor_values[3] + line_sensor_values[4]
      print(line_sensor_values)
      #if sum(clockwise_arr) > sum(counter_clockwise_arr):
        #print('clockwise')
      #else:
        #print('counterclockwise')
      counter += 1
    else:
      time.sleep(0.05)

# Run the app
#if __name__ == '__main__':
    #app.run_server(debug=True, host='0.0.0.0')
def start_app():
  #app.run_server(debug=True)
  app.server.run(port=8050, host='0.0.0.0')

def initializeThreads():
  #global zumoAngle, zumoSpeed
  #counter = 0
  #zumoAngle = 0 
  #zumoSpeed = 0
  # supress logging
  log = logging.getLogger('werkzeug')
  log.setLevel(logging.ERROR)
  
  t1 = threading.Thread(target=start_app)
  t2 = threading.Thread(target=ReceiveThread)
  t3 = threading.Thread(target=TransmitThread)   
  t1.start()
  t2.start()
  t3.start()
  t1.join()
  t2.join()
  t3.join()

#t1 = threading.Thread(target=TransmitThread)
#t2 = threading.Thread(target=ReceiveThread)
#t3 = threading.Thread(target=start_app) 
#t1.start()
#t2.start()
#t3.start()
#t1.join()
#t2.join()

if __name__ == "__main__":
  initializeThreads()