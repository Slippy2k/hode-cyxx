#!/usr/bin/env python

import os, re, tarfile, shutil
import tempfile
import zipfile

SRC_DIR = '..'

EXTRA_DIRS = [ '.', '../tools/' ]

SRCS = (
	'Makefile$',
	'.*\.h$',
	'.*\.cpp$',
	'README.txt',
)

BINS = (
	'hode.exe',
	'hode.ini',
	'icon.bmp',
	'README.txt',
	'CHANGES.txt',
	'RELEASES.yaml',
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

X86_REGS = { '_eax':'va', '_ebx':'vb', '_ecx':'vc', '_edx':'vd', '_ebp':'ve', '_edi':'vf', '_esi':'vg' }

def tidyCpp(fname):
	# only process .cpp files
	if not fname.endswith('.cpp'):
		return file(fname)
	temporary = tempfile.NamedTemporaryFile(delete=False)
	sourcefile = file(fname, 'r')
	for line in sourcefile.readlines():
		# remove lines with original executable address
		if line.startswith('// 4'):
			continue
		# replace variables named after x86 registers
		for reg, varname in X86_REGS.items():
			line = line.replace(reg, varname)
		# strip comments with 'goto address'
		ndx = line.find('// goto')
		if ndx != -1:
			line = line[:ndx] + '\n'
		temporary.write(line)
	temporary.flush()
	return temporary

src_dir = os.listdir(SRC_DIR)

filename = 'hode-%s.tar.bz2' % version
tf = tarfile.open(filename, 'w:bz2')
for name in SRCS:
	for filename in src_dir:
		if re.match(name, filename):
			entryname = 'hode-' + version + '/' + filename
			entrypath = os.path.join(SRC_DIR, filename)
			fileobj = tidyCpp(entrypath)
			tarinfo = tf.gettarinfo(name=filename, arcname=entryname, fileobj=fileobj)
			fileobj.seek(0)
			tf.addfile(tarinfo, fileobj)
tf.close()

filename = 'hode-%s-win32.zip' % version
zf = zipfile.ZipFile(filename, 'w', zipfile.ZIP_DEFLATED)
for name in BINS:
	for filename in src_dir:
		if re.match(name, filename):
			zf.write(os.path.join(SRC_DIR, filename), filename)
	for dirname in EXTRA_DIRS:
		path = os.path.join(dirname, name)
		if os.path.exists(path):
			zf.write(path, name)
zf.close()
