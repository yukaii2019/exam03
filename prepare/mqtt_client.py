
import paho.mqtt.client as paho
import time
import matplotlib.pyplot as plt
import numpy as np
import math
# https://os.mbed.com/teams/mqtt/wiki/Using-MQTT#python-client

# MQTT broker hosted on local machine
mqttc = paho.Client()

# Settings for connection
# TODO: revise host to your ip

host = "localhost"
topic = "velocity"

time = np.arange(0,10,0.1) 
velocity = np.arange(0,10,0.1)
velocity_x = np.arange(0,10,0.1)
velocity_y = np.arange(0,10,0.1)
datanum = 0

# Callbacks
def on_connect(self, mosq, obj, rc):
      print("Connected rc: " + str(rc))
def on_message(mosq, obj, msg):
    global velocity,time,datanum
    print("[Received] Topicddd: " + msg.topic + ", Message: " + str(msg.payload,encoding = "utf-8") + "\n")
    v = float(str(msg.payload,encoding = "utf-8"))
    print(v)
    if v==99999:   #ending to disconnect
        print("disconnect")
        mqttc.disconnect()
    elif(datanum<100):
        velocity[datanum] = v
        datanum = datanum+1
    elif(datanum<200):
        velocity_x[datanum] = v
        datanum = datanum+1
    elif(datanum<300):
        velocity_y[datanum] = v
        datanum = datanum+1

def on_subscribe(mosq, obj, mid, granted_qos):
      print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
      print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

# Loop forever, receiving messages
mqttc.loop_forever()

for i in range (0,100):  #PRINT TIME AND VELOCITY_horizontal
    print("time = %f , velocity = %f"%(time[i],velocity[i]))
for i in range (0,100):  #PRINT TIME AND VELOCITY_x
    print("time = %f , velocity = %f"%(time[i],velocity_x[i]))
for i in range (0,100):  #PRINT TIME AND VELOCITY_y
    print("time = %f , velocity = %f"%(time[i],velocity_y[i]))

plt.plot(time,velocity,color = 'green',linestyle = '-',label = 'horizontal')
plt.plot(time,velocity_x,color = 'blue',linestyle = '-',label = 'x')
plt.plot(time,velocity_y,color = 'green',linestyle = '-',label = 'y')
plt.xlabel('Time')
plt.ylabel('velocity')
plt.show()
