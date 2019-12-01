
import hashlib
import os
import sys

SUFFIXES = [
	'setup.dat',
	'setup.dax',
	'.paf',
	'.str',
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

def dumpPeVersionStringSection(filepath, info):
	pass

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
