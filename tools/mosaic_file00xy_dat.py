from PIL import Image

W = 512
H = 384

FILENAMES = [
'FILE0001.png',
'FILE0008.png',
'FILE0009.png',
'FILE0010.png',
'FILE0011.png',
'FILE0012.png',
'FILE0013.png',
'FILE0014.png'
]

count = len(FILENAMES)

DIRECTION = 1

if DIRECTION == 0:
	im = Image.new("RGB", (W, H * count))
	y = 0
	for name in FILENAMES:
		im.paste(Image.open(name), (0, y))
		y += H
else:
	im = Image.new("RGB", (W * count, H))
	x = 0
	for name in FILENAMES:
		im.paste(Image.open(name), (x, 0))
		x += W

im.save('hod_demo_controls.png')
