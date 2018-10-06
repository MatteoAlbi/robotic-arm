#--IMPORT
import serial
from classes import Point
#--DOC
'''
    15/05/2018  invia numeri di massimo 2 bytes
                printa a video la risposta
'''

#--VARIABILI e COSTANTI
booldebug = True
    
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

def wait_ack(ser, val=200):
    ack = -1
    
    while ack != val: 
        try:
            ack = int.from_bytes(ser.read(1), 'big')
            if booldebug: print("ACK ", ack)
        except:
            if booldebug: print("errore serial.read")
            continue

    ser.reset_input_buffer()

def send_commands(istructions, end_flag=False):

    count_commands = 0
    pen_up = 254
    end_command = 255
    
    for istr in istructions:
        if istr == '_': count_commands += 1
        else: count_commands += 2
    if end_flag: count_commands += 1

    ser.write(count_commands.to_bytes(1, 'big'))
    if booldebug: print("Count commands: ", count_commands, " ",count_commands.to_bytes(1, 'big'))

    wait_ack(ser, count_commands)
    
    for istr in istructions:
        if booldebug: print(istr)
        if istr == '_':
            ser.write(pen_up.to_bytes(1, 'big'))
            #se la penna è in alto, arduino dopo essersi spostato deve sempre abbassarla
        else:
            #--coordiante*2/3+60 in modo che il range da 0-240 passi a 60-2207
            ser.write(int(istr[0]*2/3+60).to_bytes(1, 'big'))
            ser.write(int(istr[1]*2/3).to_bytes(1, 'big'))
            if booldebug: print("b1: ", int(istr[0]*2/3+60).to_bytes(1, 'big'), "; b2: ", int(istr[1]*2/3).to_bytes(1, 'big'))
        
        if end_flag: ser.write(end_command.to_bytes(1, 'big'))

    wait_ack(ser)
    if booldebug: print("\n----------\n")

#--ELABORAZIONE
if __name__=="__main__":

    if booldebug:
        print ("start program\n\n")

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
        if booldebug: print("istructions: ",istructions)

        #gestire comando fine
        if len(path)==0:
            end_command = True
            if booldebug: print("end command")
                      
        send_commands(istructions,end_command)

    ser.close()

    if booldebug:
        print ("stop program\n\n")
