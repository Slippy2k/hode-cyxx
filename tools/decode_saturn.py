#!/usr/bin/env python

from PIL import Image
import os
import struct
import sys

def decode_img(f, fname):
	header = f.read(20)
	unk1 = struct.unpack('<I', header[0:4])[0]
	unk2 = struct.unpack('<I', header[4:8])[0]
	unk3 = struct.unpack('<I', header[8:12])[0]
	unk4 = struct.unpack('<I', header[12:16])[0]
	unk5 = struct.unpack('<I', header[16:20])[0]
	print '%d %d %d %d %d' % (unk1, unk2, unk3, unk4, unk5)
	# HOD_saturn_tokyo
	offset = 0x1CF00 - 128 + 8
	f.seek(offset)
	data = []
	w = 320
	h = 224
	image = Image.new('RGB', (w, h))
	size = w * 3 * h # 320x224
	for i in xrange(0, size, 3):
		r = ord(f.read(1))
		g = ord(f.read(1))
		b = ord(f.read(1))
		data.append((b << 16) | (g << 8) | r)
	image.putdata(data)
	image.save('HOD_Saturn_Tokyo.png')

	f.seek(0x51688 + 104 * 3 - 3 * 16)

	data = []
	w = 104
	h = 890
	image = Image.new('RGB', (w, h))
	size = w * 3 * h # 320x224
	for i in xrange(0, size, 3):
		r = ord(f.read(1))
		g = ord(f.read(1))
		b = ord(f.read(1))
		data.append((b << 16) | (g << 8) | r)
	image.putdata(data)
	image.save('HOD_Saturn_Loading.png')

# ~/Data/heart_of_darkness/DATA_saturn/00000000

for arg in sys.argv[1:]:
	f = open(arg)
	decode_img(f, arg)
