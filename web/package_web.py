#!/usr/bin/env python

import os, re, tarfile, shutil
import zipfile

VERSION = '0.1.9'

SRC_DIR = '..'

SRCS = (
	'Makefile',
	'.*h$',
	'.*cpp$',
	'README.txt',
)

BINS = (
	'hode.exe',
	'hode.ini',
	'icon.bmp',
	'README.txt',
)

src_dir = os.listdir(SRC_DIR)

filename = 'hode-%s.tar.bz2' % VERSION
tf = tarfile.open(filename, 'w:bz2')
for name in SRCS:
	for filename in src_dir:
		if re.match(name, filename):
			entry_path = os.path.join(SRC_DIR, filename)
			entry_name = 'hode-' + VERSION + '/' + filename
			tf.add(entry_path, arcname=entry_name)
tf.close()

filename = 'hode-%s.zip' % VERSION
zf = zipfile.ZipFile(filename, 'w', zipfile.ZIP_DEFLATED)
for name in BINS:
	for filename in src_dir:
		if re.match(name, filename):
			zf.write(os.path.join(SRC_DIR, filename), filename)
zf.close()
