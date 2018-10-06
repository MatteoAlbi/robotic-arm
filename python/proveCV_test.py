import cv2
import numpy as np
from braccio import stilizza
from disegna import draw_lines

img = cv2.imread('jake.jpg',0)

print(img.shape)
#img = np.float32(img)
img2 = np.zeros(img.shape)
#corners = cv2.cornerHarris(img,2,1,0.04)

for i in range(len(img)):
    for j in range(len(img[i])):
        if img[i,j] < 150:
            img2[i,j] = 255


img_prova = cv2.imread('prova.jpg',0)
prova = cv2.Canny(img_prova,100,200)
print(prova.shape)

lines = stilizza(prova,2)
draw_lines(lines,5)


