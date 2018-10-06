def fabs(x):
    if x >=0: return x
    else: return x*(-1)

class Point(object):
    
    def __init__(self, x, y):
        self.__x = x
        self.__y = y

    def __str__(self):
        return '('+str(self.__x)+', '+str(self.__y)+')'
    
    def get_x(self):
        return self.__x
    
    def get_y(self):
        return self.__y

    def set(self,x,y):
        self.__x = x
        self.__y = y


    def distance_from(self, point):
        import math
        return sqrt((self.__x - point.get_x())**2+(self.__y - point.get_y())**2)
    
    def check_tollerance(self, point, tollerance):
        #ritorna VERO se dist tra p1 e p2 <= tollerance
        import math
        
        if fabs(self.__x - point.get_x()) <= tollerance and fabs(self.__y - point.get_y()) <= tollerance:
            return True
        else: return False

class Line(object):

    def __init__(self, p1, p2):
        self.__p1 = p1
        self.__p2 = p2

    def __str__(self):
        return '[(' +str(self.__p1.get_x())+', '+str(self.__p1.get_y())+'), ('+str(self.__p2.get_x())+', '+str(self.__p2.get_y())+')]'

    def get_p1(self):
        return self.__p1

    def get_p2(self):
        return self.__p2

    def set(self,p1,p2):
        self.__p1 = p1
        self.__p2 = p2

    def length(self):
        import math
        return math.sqrt((self.__p1.get_x() - self.__p2.get_x())**2+(self.__p1.get_y() - self.__p2.get_y())**2)

    def m(self):
        p1 = self.__p1
        p2 = self.__p2
        try:
            m = (p1.get_y() - p2.get_y()) / (p1.get_x() - p2.get_x())
        except:
            m = 'inf'

        return m        

    def check_tollerance(self, line, tollerance):
        #ritorna VERO se le due linee sono simili
        if self.__p1.check_tollerance(line.get_p1(), tollerance) and self.__p2.check_tollerance(line.get_p2(), tollerance): return True
        elif self.__p1.check_tollerance(line.get_p2(), tollerance) and self.__p2.check_tollerance(line.get_p1(), tollerance): return True
        else: return False
    

