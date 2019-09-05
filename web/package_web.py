#!/usr/bin/env python

import os, re, tarfile, shutil
import zipfile

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

def getVersion():
	f = file(os.path.join(SRC_DIR, 'README.txt'))
	for line in f.readlines():
		m = re.search('Release version: (\d+\.\d+\.\d+)', line.strip())
		if m:
			return m.group(1)
	return None

version = getVersion()
assert version

src_dir = os.listdir(SRC_DIR)

filename = 'hode-%s.tar.bz2' % version
tf = tarfile.open(filename, 'w:bz2')
for name in SRCS:
	for filename in src_dir:
		if re.match(name, filename):
			entry_path = os.path.join(SRC_DIR, filename)
			entry_name = 'hode-' + version + '/' + filename
			tf.add(entry_path, arcname=entry_name)
tf.close()

filename = 'hode-%s-win32.zip' % version
zf = zipfile.ZipFile(filename, 'w', zipfile.ZIP_DEFLATED)
for name in BINS:
	for filename in src_dir:
		if re.match(name, filename):
			zf.write(os.path.join(SRC_DIR, filename), filename)
zf.close()
