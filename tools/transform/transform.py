
from PIL import Image
import sys

if sys.argv[2] == 'water':
	T = '0043ea78.bin'
	STEP = 4
else:
	assert sys.argv[2] == 'fire'
	T = '0043d960.bin'
	STEP = 4

PIC = sys.argv[1]
W = 256
H = 192

def transform(input, tdata, output, start = 0):
	for y in range(0, H):
		for x in range(0, W):
			tx = min(255, x + ord(tdata[start]))
			start += 1
			output.putpixel((x, y), input.getpixel((tx, y)))

input  = Image.open(PIC).convert('RGB')
tdata  = file(T).read()
output = Image.new('RGB', (W, H))

assert input.size[0] == W and input.size[1] == H

# duplicate the last pixels row
tdata += tdata[-256:]

step = 0
while step * STEP < W:
	transform(input, tdata, output, step * STEP)
	output.save('transform_%02d.png' % step)
	step += 1
