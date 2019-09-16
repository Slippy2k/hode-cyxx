#!/usr/bin/env python

import os
import re
import subprocess
import zipfile

ARCHIVE = 'hod_decoder-%s.zip'

SRCS = [
	'Makefile',
	'.*\.h$',
	'.*cpp$',
]

BINS = [
	'hod_decoder.exe',
	'offs_00000001.txt',
]

VERSION = subprocess.check_output('git rev-parse --short HEAD'.split()).strip()
print VERSION

name = ARCHIVE % VERSION
zf = zipfile.ZipFile(name, 'w', compression=zipfile.ZIP_DEFLATED)
src_dir = os.listdir('.')
for pattern in BINS:
	for filename in src_dir:
		if re.match(pattern, filename):
			zf.write(filename)
for pattern in SRCS:
	for filename in src_dir:
		if re.match(pattern, filename):
			arcname = 'src/' + filename
			zf.write(filename, arcname)
zf.close()
