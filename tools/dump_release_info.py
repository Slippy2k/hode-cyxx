
import hashlib
import os
import sys
import struct

SUFFIXES = [
	'setup.dat',
	'setup.dax',
	'.paf',
	'_hod.lvl',
	'_hod.sss',
	'_hod.mst',
	'hodwin32.exe',
	'hod_res.dll'
]

class ReleaseInfo(object):
	def __init__(self, name):
		self.name = name
		self.files = []
		self.version_strings = []
	def toYaml(self):
		print '- name: %s' % self.name
		if len(self.files) != 0:
			print '  files:'
			for filename, checksum in self.files:
				print '  - %s: %s' % (filename, checksum)
		if len(self.version_strings) != 0:
			print '  version_strings:'
			for key, value in self.version_strings:
				print '  - %s: %s' % (key, value)

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

def parsePeVersionStrings(f, section, start_offset):
	f.seek(start_offset + 12)
	entries_count = struct.unpack('<H', f.read(2))[0]
	entries_count += struct.unpack('<H', f.read(2))[0]
	assert entries_count == 1
	for i in range(entries_count):
		value = struct.unpack('<I', f.read(4))[0]
		offset = struct.unpack('<I', f.read(4))[0]
		f.seek(section[1] + offset)
		resource_offset = section[1] + struct.unpack('<I', f.read(4))[0] - section[2]
		resource_size = struct.unpack('<I', f.read(4))[0]

def parsePeVersion(f, section, start_offset):
	f.seek(start_offset + 12)
	entries_count = struct.unpack('<H', f.read(2))[0]
	entries_count += struct.unpack('<H', f.read(2))[0]
	assert entries_count == 1
	for i in range(entries_count):
		value = struct.unpack('<I', f.read(4))[0]
		offset = struct.unpack('<I', f.read(4))[0]
		assert value == 1
		parsePeVersionStrings(f, section, section[1] + (offset & 0x7FFFFFFF))

RESTYPE_ICON = 0x03
RESTYPE_STRING = 0x06
RESTYPE_VERSION = 0x10

def dumpPeVersionStringSection(filepath, info):
	f = file(filepath)
	sections = readPeSections(f)
	for name in sections.keys():
		if name.startswith('.rsrc'):
			section_offset = sections[name][1]
			f.seek(section_offset + 12)
			entries_count = struct.unpack('<H', f.read(2))[0]
			entries_count += struct.unpack('<H', f.read(2))[0]
			for i in range(entries_count):
				value = struct.unpack('<I', f.read(4))[0]
				offset = struct.unpack('<I', f.read(4))[0]
				if value == RESTYPE_VERSION:
					return parsePeVersion(f, sections[name], section_offset + (offset & 0x7FFFFFFF))
			break
	return None

def calculateSha1(filepath):
	h = hashlib.new('sha1')
	fp = file(filepath, 'rb')
	h.update(fp.read())
	return h.hexdigest()

def scanDirectory(dname, info):
	for dirpath, dirnames, filenames in os.walk(dname, topdown=True):
		for fn in filenames:
			for suffix in SUFFIXES:
				fname = fn.lower()
				if fname.endswith(suffix):
					fpath = os.path.join(dirpath, fn)
					checksum = calculateSha1(fpath)
					info.files.append((fname, checksum))
					if fname == 'hodwin32.exe':
						dumpPeVersionStringSection(fpath, info)
	info.files.sort()

info = ReleaseInfo(sys.argv[1])
scanDirectory(sys.argv[2], info)
info.toYaml()
