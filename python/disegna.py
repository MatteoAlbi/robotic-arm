a = [6,1,2,3,1,5]
from classes import Point, Line
import turtle


def cancella(vett):
    delete = []
    for i in range(len(vett)):
        if vett[i] < 3:
            print(vett[i])
            delete.append(i)
    print('delete: ',delete)
    print('len delete: ',len(delete))
    for i in range(len(delete)-1,-1,-1):del vett[delete[i]]

def draw_lines(lines, min_len = 4):
    import turtle

    jake = turtle.Turtle()
    jake.ht()
    
    for line in lines:
        if line.length() >= min_len:
            jake.penup()
            jake.goto(line.get_p1().get_x(), line.get_p1().get_y())
            jake.pendown()
            jake.goto(line.get_p2().get_x(), line.get_p2().get_y())

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
        


if __name__=="__main__":
    length = []
    lines = import_lines()
    #lines.append(Line(Point(100,100),Point(-100,-100)))
    for line in lines: length.append(line.length())
    for i in range(len(lines)): print(lines[i], length[i])
    print(len(lines))
    print(min(length))
    draw_lines(lines)
    
      
       

