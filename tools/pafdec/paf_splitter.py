#!/usr/bin/python

import os.path
import struct
import sys

MAGIC = 'Packed Animation File V1.0\n(c) 1992-96 Amazing Studio\n'
MAGIC_LEN = len(MAGIC)

COUNT = 50

offsets = []
f = file(sys.argv[1])
for i in range(0, COUNT):
	offs = struct.unpack('<I', f.read(4))[0]
	offsets.append(offs)

assert offsets[0] == COUNT * 4

sizes = [ 0 ] * COUNT
offset = os.path.getsize(sys.argv[1])
for i in range(COUNT - 1, -1, -1):
	if offsets[i] != 0:
		sizes[i] = offset - offsets[i]
		offset = offsets[i]

for i in range(0, COUNT):
	if offsets[i] != 0:
		print 'paf %d offset 0x%x size 0x%x' % (i, offsets[i], sizes[i])

		f.seek(offsets[i])
		assert f.read(MAGIC_LEN) == MAGIC
		f.seek(offsets[i])

		name = '%02d.paf' % i
		o = file(name, 'wb')
		o.write(f.read(sizes[i]))
		o.close()
