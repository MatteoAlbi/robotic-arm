#--IMPORT
import cv2
import numpy as np
from classes import Point, Line, Node, Graph, Queue, fabs
#--DOC

#--VARIABILI e COSTANTI
booldebug = False
booltest = False
input_image = 'fiore.jpg'
    
#--FUNZIONI e PROCEDURE
def check_bounds(x):
    if x < 0: return 0
    elif x >= len_edges: return len_edges-1
    else: return int(x)

def approximate(x):
    dec = (x%1)*10
    if dec >= 5: x = x - x%1 + 1
    else: x = x - x%1
    return int(x)

def analisi(edges, precision=5, tollerance=4):
    paths = []

    global cell_dim
    cell_dim = precision*2+1
    
    global len_edges
    len_edges = len(edges[0])
    if booldebug: print(len_edges)
    
    for i in range(len_edges):
        for j in range(len_edges):
            if edges[i,j] > 0:
                if booldebug: print(i,j),print("graph")
                path = compute_graph(edges, i, j, precision, tollerance)
                if type(path) == type([]):
                    paths.append(path)
                    
    return paths
            
def compute_graph(edges, x ,y, precision, tollerance):

    if booldebug: print(x,y)
    graph = Graph('image')
    q = Queue('nodes')
    p_node = Point(x,y)
    node = Node(p_node)
    graph.nodes.append(node)
    q.push(node)
    
    while not q.empty():
        node = q.top()
        q.pop()
        links = []

        p_node = node.get_point()
        x = p_node.get_x()
        y = p_node.get_y()
        cell = generate_cell(edges,p_node,precision)
        links = find_lines(cell,edges, x-precision, y-precision, tollerance)
        #if booldebug: print(links)
        delete(edges,cell,x-precision,y-precision)

        for p in links:
            graph.nodes.append(Node(p))
            q.push(Node(p))

        graph.manage_links(node, links)

    if len(graph.nodes)>2:
        return DPS(graph)

                
def generate_cell(edges,center,precision):
    x = center.get_x()
    y = center.get_y()
    cell = edges[check_bounds(x-precision):check_bounds(x+precision+1),check_bounds(y-precision):check_bounds(y+precision+1)]
    #if booldebug: print("cell shape: ",cell.shape)
    return cell

def delete(edges,cell, x, y):
    for i in range(x,x+11):
        for j in range(y,y+11):
            edges[i,j] = 0

def find_lines(cell, edges, i, j, tollerance):
#--data una cella controlla se vi sono linee all'interno di essa che partono dal centro
#--applico un filtro sui punti con margine di errore = tollerance
    
    cell_lines = []
    points = []
    
    if not np.array_equiv(cell,np.zeros((cell_dim,cell_dim))) and cell.shape == (cell_dim,cell_dim):
        #--ottengo lati della cella
        up = cell[0,0:(cell_dim-1)]
        down = cell[-1,1:cell_dim]
        left = cell[1:cell_dim,0]
        right = cell[0:(cell_dim-1),-1]
        #--ottengo punti sui lati
        for k in range(cell_dim-1):
            if up[k]>0: points.append(Point(i,j+k-5))
            if down[k]>0: points.append(Point(i+cell_dim-1,j+k+1))
            if left[k]>0: points.append(Point(i+k+1,j))
            if right[k]>0: points.append(Point(i+k,j+cell_dim-1))

        #--punto centrale
        center = Point(i+5,j+5)
        for p in points:
            #--calcolo il coeff m della retta passante per center e p
            #--direzione linea immaginaria in base a m
            orizontal = False
            vertical = False
            tot = 0
            
            try:
                m = (center.get_y() - p.get_y()) / (center.get_x() - p.get_x())
                if fabs(m) >= 1:
                    orizontal = True
                else:
                    vertical = True
                    
            #--se err (div per 0) stessa riga --> ORIZZONTALE m = inf
            except:
                n_punti = fabs(center.get_y() - p.get_y())
                if center.get_y()>p.get_y():
                    start = p.get_y()
                else:
                    start = center.get_y()+1
                stop = start + n_punti
                for k in range(start,stop):
                    #--scorro la retta immaginari che unisce i due punti
                    center_x = center.get_x()
                    #--calcolo somma con celle in posiz +-1
                    tot = tot + edges[check_bounds(center_x-1), k]  
                    tot = tot + edges[center_x, k]
                    tot = tot + edges[check_bounds(center_x+1), k]
            
            
#--se verticale scorro sull'asse x
            if vertical:
                n_punti = fabs(center.get_x() - p.get_x())
                if center.get_x()>p.get_x():
                    start = p.get_x()
                else:
                    start = center.get_x()+1
                stop = start + n_punti
                
                for k in range(start,stop):
                    #--scorro la retta immaginari che unisce i due punti
                    center_y = approximate(m * k - m * center.get_x() + center.get_y())
                    #--calcolo somma con celle in posiz +-1
                    tot = tot + edges[k, check_bounds(center_y-1)]
                    tot = tot + edges[k, center_y]
                    tot = tot + edges[k, check_bounds(center_y+1)]                             

#--se orizzontle scorro sull'asse y
            if orizontal:
                n_punti = fabs(center.get_y() - p.get_y())
                if center.get_y()>p.get_y():
                    start = p.get_y()
                else:
                    start = center.get_y()+1
                stop = start + n_punti
                
                for k in range(start,stop):
                    #--scorro la retta immaginari che unisce i due punti
                    center_x = approximate((k - center.get_y()) / m + center.get_x())
                    #--calcolo somma con celle in posiz +-1
                    tot = tot + edges[check_bounds(center_x-1), k]
                    tot = tot + edges[center_x, k]
                    tot = tot + edges[check_bounds(center_x+1), k]

            #--se somma > del totale del 90% dei punti tra center e p --> new line
            if tot/n_punti >= 255*80/100:

                new_line = Line(center,p)
                repeated_line = False
            #--controllo se è già scoperta/simile ad un'altra
                for line in cell_lines:
                    if new_line.check_tollerance(line, tollerance):
                        repeated_line = True
                        break

                if not repeated_line:
                    cell_lines.append(new_line)

    #--salvo come nodi link i punti cge formano una linea col centro
    links = []
    for line in cell_lines:
        links.append(line.get_p2())

    return links

def DPS(graph):
    if booldebug: print("DPS start")
    if booldebug: print("n nodes ",len(graph.nodes))
    q = Queue("nodi")
    path = []
    back = False

    #--inizializzo coda e punto di partenza
    i_node = 0
    graph.nodes[i_node].grey()
    q.push(i_node)
    path.append(graph.nodes[i_node].get_point())

    #--finchè coda non vuota 
    while not q.empty():
        #--estraggo nodo
        i_node = q.top()
        if booldebug: print(i_node,", links: ",len(graph.nodes[i_node].get_links()))
        new_node = False
        #--controllo nodi adiacenti
        for p in graph.nodes[i_node].get_links():
            j_node = graph.find_node(p)
            #--se c'è ne uno non esplorato(bianco, state = 0)
            if graph.nodes[j_node].get_state() == 0:
                new_node = True
                #--lo inserisco nella coda e vadoavanti
                graph.nodes[j_node].grey()
                q.push(j_node)
                #se sto tornando indietro nell'albero aggiungo simbolo dii salto nel path
                if back:
                    back = False
                    path.append('_')#simbolo salto
                    path.append(graph.nodes[i_node].get_point())
                path.append(graph.nodes[j_node].get_point())
                
                break
        #-- se non ci sono nuovi nodi torno indietro nella coda al nodo precedente   
        if not new_node:
            graph.nodes[i_node].black();
            q.pop()
            back = True

    if booldebug: print("DPS ended")
    return path

def merge(vett):
    if booldebug:
        for v in vett:
            print(v[0],v[-1])
        print("start")
        
    path = []
    sort = []
    i=0
    sort.append(vett[i])
    
    del vett[i]

    while vett != []:
        next_p = -1
        if booldebug:
            for p in sort[i]: print (p)
            print("-----------------")
        if booldebug: print("last: ",sort[i][-1])
        for j in range(len(vett)):
            try:
                if sort[i][-1].check_tollerance(vett[j][0],20):
                    next_p = j
                    break
            except:
                continue
        if booldebug: print("next_p: ",next_p)
        if next_p >= 0:
            sort.append(vett[next_p])
            del vett[next_p]
            i+=1
        else:
            sort.append("_")
            sort.append(vett[0])
            del vett[0]
            i+=2

    for v in sort:
        if v == "_":
            path.append(v)
        else:
            for p in v:
                path.append(p)

    return path

def export_path(path):
    fout = open('path.txt','w')

    for p in path:
        if p == "_":
            fout.write("_\n")
        else:
            fout.write(str(p.get_x()) + ',' + str(p.get_y()) + '\n')

    fout.close()

def draw_path(path):
    import turtle

    jake = turtle.Turtle()
    jake.ht()
    jake.penup()

    for p in path:
        if p == '_':
            jake.penup()
        else:
            jake.goto(p.get_x(), p.get_y())
            if not jake.isdown(): jake.pendown()

#--ELABORAZIONE
if __name__=="__main__":

    if booldebug or booltest:
        print ("start program\n\n")
        
    img = cv2.imread(input_image)

    edges = cv2.Canny(img,150,200)
    cv2.imshow("edges", edges)

    paths = analisi(edges)
    path = merge(paths)
    draw_path(path)
    export_path(path)
    
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    if booldebug or booltest:
        print ("stop program\n\n")
