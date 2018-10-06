#--IMPORT
import serial
#--DOC
'''
    15/05/2018  invia numeri di massimo 2 bytes
                printa a video la risposta
'''

#--VARIABILI e COSTANTI
booldebug = False
end = False
    
#--FUNZIONI e PROCEDURE


#--ELABORAZIONE
if __name__=="__main__":

    if booldebug:
        print ("start program\n\n")

    ser = serial.Serial('COM3', 9600);
    print(ser)
        
    while not end:
        a = input("mex: (e to end)")
        try:
            b = int(a).to_bytes(2, 'big')
            print('inviato ',b)
            ser.write(b)
            print('ricevuto ',int.from_bytes(ser.read(2), 'big'))
        except:
            if a == 'e': end = True

    ser.close()

    if booldebug:
        print ("stop program\n\n")
