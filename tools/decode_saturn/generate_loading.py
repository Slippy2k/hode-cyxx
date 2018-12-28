
import os
import sys
from PIL import Image

y = 0

# 320x224
background_frame = 'HOD_Saturn_Tokyo.png'
# 104x102
loading_frames = 'HOD_Saturn_Loading.png'

LOADING_W = 103
LOADING_H = 48
LOADING_H = [ 49, 105, 161, 217, 273, 329, 385, 441, 497, 553, 609, 665, 721, 777, 833, 889 ]

loading_img = Image.open(loading_frames)
img = Image.open(background_frame).convert('RGBA')
counter = 0
y = 0
while y < loading_img.size[1] and counter < len(LOADING_H):
	y2 = LOADING_H[counter]
	overlay = loading_img.copy().crop((0, y, LOADING_W, y2))
	h = y2 - y
	img.paste(overlay, (116, 168 - h + 1))
	fname = 'saturn_loading_%02d.png' % counter
	counter += 1
	img.save(fname)
	y = y2
