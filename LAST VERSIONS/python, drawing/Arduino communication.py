#--IMPORT
import serial
import cv2
import numpy as np
from classes import Point
#--DOC
'''
    15/05/2018  invia numeri di massimo 2 bytes
                printa a video la risposta
'''

#--VARIABILI e COSTANTI
booldebug = True
input_file = 'config.txt'
    
#--FUNZIONI e PROCEDURE

def import_path():
    path = []
    fin = open('path.txt','r')
    if booldebug: print ("importing path...")

    l = fin.readline().strip()
    while l != "":
        if l == "_":
            path.append("_")
        else:
            v = l.split(",")
            path.append([int(v[0]),int(v[1])])
        l = fin.readline().strip()
    
    fin.close()
    if booldebug:
        for i in path: print(i)
                        
    return path

def wait_ack(ser, val=253):
    ack = -1
    
    while ack != val: 
        try:
            ack = int.from_bytes(ser.read(1), 'big')
            if booldebug: print("ACK ", ack)
        except:
            if booldebug: print("errore serial.read")
            continue

    ser.reset_input_buffer()

def send_commands(istructions, redim, end_flag=False):
    vett = []
    pen_up = 254
    end_command = 255
    
    for istr in istructions:
        if istr == '_': vett.append(pen_up)
        else: vett.append(int(istr[0]*redim)+80), vett.append(int(istr[1]*redim))
    if end_flag: vett.append(end_command)

    if booldebug: print(vett)

    ser.write(len(vett).to_bytes(1, 'big'))
    if booldebug: print("Count commands: ", len(vett), " ",len(vett).to_bytes(1, 'big'))

    wait_ack(ser, len(vett))

    for n in vett:
        ser.write(n.to_bytes(1, 'big'))
        if booldebug: print(n.to_bytes(1, 'big'))
    
    wait_ack(ser)
    
    if booldebug: print("\n----------\n")

#--ELABORAZIONE
if __name__=="__main__":

    if booldebug:
        print ("start program\n\n")

    fin = open(input_file,'r')
    input_image = fin.readline().strip()
    fin.close()
    if booldebug: print(input_image)

    img = cv2.imread(input_image)
    redim = 160/img.shape[0]

    path = import_path()

    ser = serial.Serial('COM3', 9600);
    print(ser)
    end_command = False

    wait_ack(ser)
    if booldebug: print("device ready\n")
    
    while len(path)>0:
        L = min(8,len(path))
        istructions = path[:L]
        del path[:L]
        #if booldebug: print("istructions: ",istructions)

        #gestire comando fine
        if len(path)==0:
            end_command = True
            if booldebug: print("end command")
                      
        send_commands(istructions, redim, end_command)

    ser.close()

    if booldebug:
        print ("stop program\n\n")
