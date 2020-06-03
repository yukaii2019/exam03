import serial
import time
import matplotlib.pyplot as plt
import numpy as np
import paho.mqtt.client as paho
# MQTT broker hosted on local machine
mqttc = paho.Client()
# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())
s.write("ATMY 0x165\r\n".encode())
char = s.read(3)
print("Set MY 0x165.")
print(char.decode())
s.write("ATDL 0x265\r\n".encode())
char = s.read(3)
print("Set DL 0x265.")
print(char.decode())
s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())
s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())
s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())
s.write("ATCN\r\n".encode())
char = s.read(4)             
print("Exit AT mode.")
print(char.decode())
print("start sending RPC")

volocity = np.arange(0,10,0.1)
t = np.arange(0,10,0.1)  #time

s.write("/GET_VOLOCITY/run\r".encode())
for i in range(0,100):
    char = s.read(9)
    #print(char.decode())
    volocity[i] = float(char.decode())
    print(volocity[i])

# Settings for connection
host = "localhost"
topic= "velocity"
port = 1883
# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

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

#publish
for i in range(0,100):
    mesg = str(volocity[i])
    mqttc.publish(topic, mesg)
    print(mesg)
    time.sleep(0.1)
mesg = "99999"  #ending to disconnect
mqttc.publish(topic, mesg)
s.close()

