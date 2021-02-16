#!/usr/bin/env python

import datetime
import os, re, tarfile, shutil
import tempfile
import time
import zipfile

SRC_DIR = '..'

EXTRA_DIRS = [ '.', '../tools/' ]

SRC = (
	'Makefile$',
	'.*\.h$',
	'.*\.cpp$',
	'README.txt',
	'CHANGES.txt',
	'RELEASES.yaml',
)

BIN_WIN32 = (
	'hode.exe',
	'hode.ini',
	'icon.bmp',
	'README.txt',
	'CHANGES.txt',
	'RELEASES.yaml',
)

BIN_PSP = (
	'EBOOT.PBP',
	'hode.ini',
	'README.txt',
	'CHANGES.txt'
	'RELEASES.yaml',
)

BIN_WII = (
	'hode.dol',
	'icon.png',
	'meta.xml',
)

WII = False

def getVersion():
	f = file(os.path.join(SRC_DIR, 'README.txt'))
	for line in f.readlines():
		m = re.search('Release version: (\d+\.\d+\.\d+\w?)', line.strip())
		if m:
			return m.group(1)
	return None

version = getVersion()
assert version

timetuple = datetime.date.today().timetuple()
timestamp = time.mktime(timetuple)

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
		# strip comments with x86 registers
		ndx = line.find(' // _e')
		if ndx != -1:
			line = line[:ndx] + '\n'
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

def getTarInfo(tf, path, entryname, timestamp):
	fileobj = tidyCpp(path)
	tarinfo = tf.gettarinfo(name=entryname, arcname=entryname, fileobj=fileobj)
	tarinfo.mtime = timestamp
	fileobj.seek(0)
	return (tarinfo, fileobj)

filename = 'hode-%s.tar.bz2' % version
prefix = 'hode-%s/' % version
tf = tarfile.open(filename, 'w:bz2')
for name in SRC:
	for filename in src_dir:
		if re.match(name, filename):
			args = getTarInfo(tf, os.path.join(SRC_DIR, filename), prefix + filename, timestamp)
			tf.addfile(*args)
	for dirname in EXTRA_DIRS:
		path = os.path.join(dirname, name)
		if os.path.exists(path):
			args = getTarInfo(tf, path, prefix + name, timestamp)
			tf.addfile(*args)
tf.close()

def getZipDataStr(zf, path, name, t):
	zi = zipfile.ZipInfo(name)
	zi.date_time = t[0:6]
	return (zi, file(path).read())

zipfiles = [ ('hode-%s-win32.zip', BIN_WIN32), ('hode-%s-psp.zip', BIN_PSP) ]
for f in zipfiles:
	filename = f[0] % version
	zf = zipfile.ZipFile(filename, 'w', zipfile.ZIP_DEFLATED)
	for name in f[1]:
		for filename in src_dir:
			if re.match(name, filename):
				args = getZipDataStr(zf, os.path.join(SRC_DIR, filename), filename, timetuple)
				zf.writestr(*args, compress_type=zipfile.ZIP_DEFLATED)
		for dirname in EXTRA_DIRS:
			path = os.path.join(dirname, name)
			if os.path.exists(path):
				args = getZipDataStr(zf, path, name, timetuple)
				zf.writestr(*args, compress_type=zipfile.ZIP_DEFLATED)
	zf.close()

if WII:
	filename = 'hode-%s-wii.zip' % version
	zf = zipfile.ZipFile(filename, 'w', zipfile.ZIP_DEFLATED)
	zf.write(os.path.join(SRC_DIR, 'hode.dol'), 'apps/hode/boot.dol')
	zf.write(os.path.join(SRC_DIR, 'hode.ini'), 'apps/hode/hode.ini')
	zf.write(os.path.join(SRC_DIR + '/dists/wii/', 'meta.xml'), 'apps/hode/meta.xml')
	zf.write(os.path.join(SRC_DIR + '/dists/wii/', 'icon.png'), 'apps/hode/icon.png')
	zf.writestr('hode/COPY YOUR DATA FILES HERE', '')
	zf.close()
