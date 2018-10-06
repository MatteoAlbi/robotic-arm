import cv2
import numpy as np
from braccio import *
from disegna import *

img_prova = cv2.imread('prova.jpg')
prova = cv2.Canny(img_prova,100,200)
print(prova.shape)

lines = stilizza(prova,2)
draw_lines(lines,5)


