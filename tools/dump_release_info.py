
import hashlib
import os
import re
import subprocess
import struct
import sys

SUFFIXES = [
	'setup.dat',
	'setup.dax',
	'.paf',
	'_hod.lvl',
	'_hod.sss',
	'_hod.mst',
	'hod.dem',
	'hodwin32.exe',
	'hod_res.dll'
]

WIN32_EXECUTABLE = 'hodwin32.exe'
PSX_EXECUTABLE_RE = r'sl\w\w_\d+\.\d+'

class ReleaseInfo(object):
	def __init__(self, name):
		self.name = name
		self.files = []
		self.win32_version_info = []
		self.win32_securom = 'false'
		self.psx_exe_header = {}
	def toYaml(self):
		print '- name: %s' % self.name
		if len(self.files) != 0:
			print '  files:'
			for filename, checksum in self.files:
				print '  - %s: %s' % (filename, checksum)
				if filename == WIN32_EXECUTABLE:
					print '    securom: ' + self.win32_securom
					if len(self.win32_version_info) != 0:
						print '    version_info:'
						for key, value in self.win32_version_info:
							print '    - %s: %s' % (key, value)
				if filename in self.psx_exe_header.keys():
					for key, value in self.psx_exe_header[filename]:
						print '    - %s: %s' % (key, value)

def readPeSections(f):
	assert f.read(2) == 'MZ'
	f.seek(58, os.SEEK_CUR)
	pe_offset = struct.unpack('<I', f.read(4))[0]
	f.seek(pe_offset)
	assert f.read(4) == 'PE\0\0'
	f.seek(2, os.SEEK_CUR)
	sections_count = struct.unpack('<H', f.read(2))[0]
	f.seek(12, os.SEEK_CUR)
	extra_size = struct.unpack('<H', f.read(2))[0]
	f.seek(extra_size + 2, os.SEEK_CUR)
	sections = {}
	for i in range(sections_count):
		section_name = f.read(8)
		f.seek(4, os.SEEK_CUR)
		section_addr = struct.unpack('<I', f.read(4))[0]
		section_size = struct.unpack('<I', f.read(4))[0]
		section_offset = struct.unpack('<I', f.read(4))[0]
		f.seek(16, os.SEEK_CUR)
		sections[section_name] = (section_size, section_offset, section_addr)
	return sections

def dumpPeVersionStringSection(filepath, info):
	f = file(filepath)
	sections = readPeSections(f)
	for name in sections.keys():
		if name.startswith('.cms_'):
			info.win32_securom = 'true'
		elif name.startswith('.rsrc'):
			args = [ 'wine', 'decode_pe', filepath ]
			p = subprocess.Popen(args, stdout=subprocess.PIPE)
			for line in p.stdout.readlines():
				s = line.strip()
				sep = s.index(':')
				if sep != -1:
					name = s[:sep].strip()
					value = s[sep + 1:].strip()
					if name == 'LegalCopyright' and ord(value[0]) == 0xA9:
						value = '(c)' + value[1:]
					info.win32_version_info.append((name, value))

def dumpPsxExecutableInfo(filepath, filename, info):
	f = file(filepath)
	assert f.read(8) == 'PS-X EXE'
	f.seek(0x18)
	header = []
	header.append(('text_addr', '0x%x' % struct.unpack('<I', f.read(4))[0]))
	header.append(('text_size', '%d' % struct.unpack('<I', f.read(4))[0]))
	f.seek(0x4C)
	copyright = f.read(256)
	for i, c in enumerate(copyright):
		if ord(c) == 0:
			header.append(('copyright', copyright[:i]))
			break
	info.psx_exe_header[filename] = header

def calculateSha1(filepath):
	h = hashlib.new('sha1')
	fp = file(filepath, 'rb')
	h.update(fp.read())
	return h.hexdigest()

def matchName(fn):
	for suffix in SUFFIXES:
		fname = fn.lower()
		if fname.endswith(suffix):
			return True
	return re.match(PSX_EXECUTABLE_RE, fn.lower()) != None

def scanDirectory(dname, info):
	for dirpath, dirnames, filenames in os.walk(dname, topdown=True):
		for fn in filenames:
			if matchName(fn):
				fname = fn.lower()
				fpath = os.path.join(dirpath, fn)
				checksum = calculateSha1(fpath)
				info.files.append((fname, checksum))
				if fname == WIN32_EXECUTABLE:
					dumpPeVersionStringSection(fpath, info)
				elif re.match(PSX_EXECUTABLE_RE, fname):
					dumpPsxExecutableInfo(fpath, fname, info)
	info.files.sort()

info = ReleaseInfo(sys.argv[1])
scanDirectory(sys.argv[2], info)
info.toYaml()
