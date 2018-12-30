from PIL import Image

W = 256
H = 192

NUM = 25

im = Image.new("RGB", (W, H * NUM))
y = 0

for i in range(0, NUM):
	name = '%02d.bmp' % i
	im.paste(Image.open(name), (0, y))
	y += H

im.save('HoD_Saturn_Level1.png')

