
from PIL import Image

T = '0043ea78.bin'
PIC = 'pwr1_hod_01_0.bmp'
W = 256
H = 192

STEPS = 16

def transform(input, tdata, output, start = 0):
	for y in range(0, H):
		for x in range(0, W):
			offset = x + ord(tdata[start % len(tdata)])
			start += 1
			output.putpixel((x, y), input.getpixel((offset % W, y)))

input  = Image.open(PIC).convert('RGB')
tdata  = file(T).read()
output = Image.new('RGB', (W, H))

assert input.size[0] == W and input.size[1] == H

for step in range(0, STEPS):
	transform(input, tdata, output, (step + 1) * 4)
	output.save('transform_%02d.png' % step)
	# output.show()
