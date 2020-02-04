
import os
import struct
import sys

assert len(sys.argv) == 2
f = open(sys.argv[1], 'rb')
while True:
	signature = f.read(4)
	if not signature:
		break
	assert signature == 'REC1'
	total_size    = struct.unpack('<I', f.read(4))[0]
	random_seed   = struct.unpack('<I', f.read(4))[0]
	keymask_size  = struct.unpack('<I', f.read(4))[0]
	level         = ord(f.read(1))
	checkpoint    = ord(f.read(1))
	difficulty    = ord(f.read(1))
	shuffle_count = ord(f.read(1)) # _rnd.initTable
	f.read(4) # ptr to actionKeyMask
	f.read(4) # ptr to directionKeyMask
	print 'rand 0x%x size %d tblsize %d level %d checkpoint %d' % (random_seed, total_size, keymask_size, level, checkpoint)
	assert keymask_size * 2 == total_size - 8 - 20
	f.seek(keymask_size * 2, os.SEEK_CUR)
