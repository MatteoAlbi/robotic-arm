#--IMPORT
import cv2
import numpy as np
#--DOC

#--VARIABILI e COSTANTI
booldebug = False
booltest = True
file_test = 'prova.txt'
input_image = 'prova.jpg'


#cell = np.zeros((cell_dim,cell_dim))
    
#--FUNZIONI e PROCEDURE

def check_iter(a):
    return min(len_edges,a+cell_dim)

def check_bounds(x):
    if x < 0: return 0
    elif x >= len_edges: return len_edges-1
    else: return int(x)

def approximate(x):
    dec = (x%1)*10
    if dec >= 5: x = x - x%1 + 1
    else: x = x - x%1
    return int(x)

def check_redundance(cell_lines):
    #elimina le linee sottogruppo di altre
    #con m uguale e lunghezze diverse --> possibile solo su lati
    #non necessità di controllare se sono linee coincidenti o diverse
    delete = []
    
    for i in range(len(cell_lines)):
        for j in range(len(cell_lines)):
            if cell_lines[i].m() == cell_lines[j].m() and i != j:
                if cell_lines[i].lenght() < cell_lines[j].lenght(): delete.append(i)
                elif cell_lines[i].lenght() > cell_lines[j].lenght(): delete.append(j)

    for i in range(len(delete)-1,-1,-1): del cell_lines[delete[i]]
            
            


def find_lines(cell, edges, i, j, tollerance):
#--data una cella controlla se vi sono linee all'interno di essa
#--applico un filtro sui punti con margine di errore = tollerance
    import numpy as np
    from classes import Point, Line, fabs
    
    cell_lines = []
    points = []
    dim = cell.shape[0]
    
    if not np.array_equiv(cell,np.zeros((cell_dim,cell_dim))) and cell.shape == (cell_dim,cell_dim):

        #--ottengo lati della cella
        up = cell[0,0:(cell_dim-1)]
        down = cell[-1,1:cell_dim]
        left = cell[1:cell_dim,0]
        right = cell[0:(cell_dim-1),-1]
        #--ottengo punti sui lati
        for k in range(cell_dim-1):
            if up[k]>0: points.append(Point(i,j+k))
            if down[k]>0: points.append(Point(i+cell_dim-1,j+k+1))
            if left[k]>0: points.append(Point(i+k+1,j))
            if right[k]>0: points.append(Point(i+k,j+cell_dim-1))

        if booldebug: print('POINTS:')
        if booldebug:
            for point in points: print(point)

        for p1 in points:
            for p2 in points:
                if booldebug: print('\n----------\n')
                if booldebug: print('points: ',p1,' ',p2)
                if not p1.check_tollerance(p2, tollerance):
                    #--calcolo il coeff m della retta passante per loro 2
                    #--direzione linea immaginaria 
                    orizontal = False
                    vertical = False
                    tot = 0
                    
                    try:
                        m = (p1.get_y() - p2.get_y()) / (p1.get_x() - p2.get_x())
                        if booldebug: print('m: ',m)
                        if fabs(m) >= 1:
                            orizontal = True
                            if booldebug: print('ORIZONTAL\n')
                        else:
                            vertical = True
                            if booldebug: print('VERTICAL\n')
                            
                    #--se err (div per 0) stessa riga --> ORIZZONTALE m = inf
                    except:
                        n_punti = fabs(p1.get_y() - p2.get_y())+1
                        start = min(p1.get_y(),p2.get_y())
                        stop = start + n_punti
                        if booldebug: print('n_punti: ', n_punti, ' range: ', start, stop)
                        for k in range(start,stop):
                            #--scorro la retta immaginari che unisce i due punti
                            centre_x = p1.get_x()
                            #if booldebug: print('point: ', centre_x, k)
                            #--calcolo somma con celle in posiz +-2
                            tot = tot + edges[centre_x, k]
                            tot = tot + edges[check_bounds(centre_x+1), k]
                            #tot = tot + edges[check_bounds(centre_x+2), k]
                            tot = tot + edges[check_bounds(centre_x-1), k]
                            #tot = tot + edges[check_bounds(centre_x-2), k]
                        #orizontal = True
                        #m = 'inf'                    
                    
#--se verticale scorro sull'asse x
                    if vertical:
                        n_punti = fabs(p1.get_x() - p2.get_x())+1
                        start = min(p1.get_x(),p2.get_x())
                        stop = start + n_punti
                        if booldebug: print('n_punti: ', n_punti, ' range: ', start, stop)

                        for k in range(start,stop):
                            #--scorro la retta immaginari che unisce i due punti
                            centre_y = approximate(m * k - m * p1.get_x() + p1.get_y())
                            
                            #--calcolo somma con celle in posiz +-1
                            #tot = tot + edges[k, check_bounds(centre_y-2)]
                            tot = tot + edges[k, check_bounds(centre_y-1)]
                            tot = tot + edges[k, centre_y]
                            tot = tot + edges[k, check_bounds(centre_y+1)]
                            #tot = tot + edges[k, check_bounds(centre_y+2)]                               

#--se orizzontle scorro sull'asse y
                    if orizontal:
                        n_punti = fabs(p1.get_y() - p2.get_y())+1
                        start = min(p1.get_y(),p2.get_y())
                        stop = start + n_punti
                        if booldebug: print('n_punti: ', n_punti, ' range: ', start, stop)

                        for k in range(start,stop):
                            #--scorro la retta immaginari che unisce i due punti
                            if m == 'inf': centre_x = p1.get_x()
                            else: centre_x = approximate((k - p1.get_y()) / m + p1.get_x())
   
                            tot = tot + edges[centre_x, k]
                            tot = tot + edges[check_bounds(centre_x+1), k]
                            #tot = tot + edges[check_bounds(centre_x+2), k]
                            tot = tot + edges[check_bounds(centre_x-1), k]
                            #tot = tot + edges[check_bounds(centre_x-2), k]
                            
                    if booldebug: print('tot ',tot)
                    if tot/n_punti >= 255*90/100:

                        new_line = Line(p1,p2)
                        repeated_line = False

                        for line in cell_lines:
                            if new_line.check_tollerance(line, tollerance):
                                repeated_line = True
                                break

                        if not repeated_line:
                            cell_lines.append(new_line)
                            if booldebug: print('NEW LINE')
                        elif booldebug:
                            print('ALREADY EXIST')
                    elif booldebug: print('NO LINE')

                elif booldebug: print('PUNTI VICINI')
                            
    elif booldebug: print('EMPTY')

    #check_redundance(cell_lines) DA RIVEDERE
    
    return cell_lines

def export_lines(lines):
    from classes import Point, Line
    fout = open('lines.txt','w')

    for line in lines:
        fout.write(str(line.get_p1().get_x()) + '-' + str(line.get_p1().get_y()) + '|' + str(line.get_p2().get_x()) + '-' +  str(line.get_p2().get_y())+'\n')

    fout.close()

def import_lines():
    from classes import Point, Line
    fin = open('lines.txt','r')
    lines = []

    riga = fin.readline().strip()
    while riga != '':
        vett = riga.split('|')
        v1 = vett[0]
        v2 = vett[1]

        p1 = v1.split('-')
        p2 = v2.split('-')

        line = Line(Point(int(p1[0]),int(p1[1])),Point(int(p2[0]),int(p2[1])))
        lines.append(line)

        riga = fin.readline().strip()

    fin.close()
    return lines

def stilizza(edges,tollerance=3):
    from classes import Point, Line, fabs
    import cv2
    import numpy as np

    global cell_dim
    cell_dim = 8
    
    tot_lines = []
    
    cv2.imshow("edges", edges)

    global len_edges
    len_edges = len(edges[0])

    for i in range(0,len_edges,cell_dim):
        for j in range(0,len_edges,cell_dim):
            cell = edges[i:i+cell_dim,j:j+cell_dim]
            if booldebug: print(cell)
            tot_lines = tot_lines + find_lines(cell, edges, i, j, tollerance)

    return tot_lines

#--ELABORAZIONE
if __name__=="__main__":

    if booldebug:
        print ("start program\n\n")
        
    img = cv2.imread(input_image)

    edges = cv2.Canny(img,100,150)
    cv2.imshow("edges", edges)

    if not booltest:
        #lines = stilizza(edges)
        print('\n\nLINES: ')
        for line in lines: print(line)
        export_lines(lines)
                
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    if booldebug:
        print ("stop program\n\n")
