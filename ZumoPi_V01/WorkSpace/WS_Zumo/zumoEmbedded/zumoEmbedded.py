# Run this app with `python app.py` and
# visit http://192.168.0.102:8050/ in your web browser.

########## Imports ############

import dash
from dash import html
from dash import dcc
import dash_daq as daq
import serial
import time, math
import threading
import json
import plotly.graph_objs as go
import plotly
import logging

########## Global variables ##########

# scatter plot vars
X = [0]
Y = [0]

# barchart vars
prox_sensor_names = ['Left-Left', 'Left', 'Center', 'Right', 'Right-Right']
line_sensor_values = [0] * 5

# direction discovery vars
clockwise_arr = [0] * 100
counter_clockwise_arr = [0] * 100
counter = 0

# joystick vars
zumoAngle = 0
zumoSpeed = 0

# button vars
auto_flag = 0

########## Serial Connection ##########

#ser = serial.Serial('/dev/ttyACM0')  # open serial port
#ser = serial.Serial('COM3', baudrate = 9600, timeout = 1)

# App Initialization and definition

app = dash.Dash(__name__)

app.layout = html.Div(children=[daq.Joystick(id='my-joystick', label="Zumo Joystick", angle=0, size=100),
                                html.Div(id='joystick-output'),
                                html.Div(className='row', children=[html.Button('Automatic', id='auto_button', n_clicks=0),
                                                                    html.Button('Manual', id='manual_button', n_clicks=0),
                                                                    html.Div(id='Button output'),
                                                                    html.Div([dcc.Graph(id = 'live-barchart', animate = True),
                                                                              dcc.Interval(id = 'barchart-update', interval = 1000, n_intervals = 0),
                                                                    html.Div([dcc.Graph(id = 'live-graph', animate = True),
                                                                              dcc.Interval(id = 'graph-update', interval = 1000, n_intervals = 0)])
])])])

########## Callback Functions ##########

# Callback function - Scatter plot (not finished, currently just a straight line)
@app.callback(dash.dependencies.Output('live-graph', 'figure'),
              dash.dependencies.Input('graph-update', 'n_intervals'))

def update_graph_scatter(n):
    X.append(X[-1]+1)
    Y.append(Y[-1]+1)

    data = go.Scatter(x=list(X), y=list(Y), name='Scatter', mode='lines+markers')
    return {'data': [data],
            'layout': go.Layout(xaxis=dict(range=[min(X), max(X)]),
                                yaxis=dict(range=[min(Y), max(Y)]))}

# Callback function - Barchart
@app.callback(dash.dependencies.Output('live-barchart', 'figure'),
              [dash.dependencies.Input('barchart-update', 'n_intervals')])

def update_graph_bar(n):
  clrs = list()
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

# Callback function - Mode button clicks
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

# Callback function - Joystick
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

# Entry point of the transmit thread
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
    msg = str(auto_flag) + ';' + direction + '.' + str(joyX) + ',' +str(joyY) + '\r\n'
    ser.write(msg.encode('ascii'))
    time.sleep(0.1)

# Entry point of the receive thread
def ReceiveThread():
  global line_sensor_values, clockwise_arr, counter_clockwise_arr, counter
  while(ser.isOpen):
    if (ser.in_waiting > 0):
      sensor = ser.readline().decode('ascii')
      data = json.loads(sensor)
      for i in range(len(data["line_sensor_values"])):
        line_sensor_values[i] = data["line_sensor_values"][i]
      counter_clockwise_arr[counter%100] = line_sensor_values[0] + line_sensor_values[1]
      clockwise_arr[counter%100] = line_sensor_values[3] + line_sensor_values[4]
      print(line_sensor_values)
      counter += 1
    else:
      time.sleep(0.05)

# Entry point of the app thread
def start_app():
  #app.run_server(debug=True, host='0.0.0.0')
  app.server.run(port=8050, host='0.0.0.0')

def initializeThreads():
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

if __name__ == "__main__":
  initializeThreads()
