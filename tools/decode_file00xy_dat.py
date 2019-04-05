#!/usr/bin/env python

from PIL import Image
import os
import struct
import sys

def decode_img(f, fname):
	header = f.read(50)
	w = struct.unpack('<H', header[0:2])[0]
	h = struct.unpack('<H', header[2:4])[0]
	print '%d x %d' % (w, h)
	assert (w == 512 and h == 384)
	image = Image.new('P', (w, h))
	rgba = f.read(256 * 4)
	palette = []
	for i in xrange(0, 256 * 4, 4):
		palette.append(ord(rgba[i + 2]))
		palette.append(ord(rgba[i + 1]))
		palette.append(ord(rgba[i + 0]))
		image.putpalette(palette)
	image.putdata(f.read(w * h))
	image = image.transpose(Image.FLIP_TOP_BOTTOM)
	image.save(os.path.splitext(fname)[0] + '.png')

for arg in sys.argv[1:]:
	f = open(arg)
	decode_img(f, arg)
