
AC_COEFFS_TXT = 'mdec_ac_coeffs.txt'

AC_COEFFS_COUNT = 111

NODES = []

class Tree(object):
	def __init__(self):
		self.left  = None
		self.right = None
		self.value = None
		global NODES
		self.num = len(NODES)
		NODES.append(self)

HUFFMAN_TREE = Tree()

def addCode(code, value):
	global HUFFMAN_TREE
	node = HUFFMAN_TREE
	for bit in code:
		if bit == '1':
			if not node.right:
				node.right = Tree()
			node = node.right
		elif bit == '0':
			if not node.left:
				node.left = Tree()
			node = node.left
		else:
			assert bit == 's' or bit == 't'
			node.value = value

def dumpNode(node):
	if not node.left and not node.right:
		assert node.value != 0
		print '\t{ -1, -1, 0x%04x},' % (node.value)
	else:
		assert not node.value
		if node.left:
			left = node.left.num
		else:
			left = -1
		if node.right:
			right = node.right.num
		else:
			right = -1
		print '\t{ %d, %d, 0x0000 },' % (left, right)

# collect variable length codes
vlc = [] # code, non-zero values, zero values
for line in file(AC_COEFFS_TXT).readlines():
	values = line.strip().split()
	assert len(values) >= 3
	code = ''.join(values[0:-2])
	nonzero = int(values[-2])
	zero = int(values[-1])
	assert (nonzero + zero) != 0
	vlc.append( (code, ((nonzero << 8) | zero)) )
assert len(vlc) == AC_COEFFS_COUNT

# 000001 Escape
vlc.append( ('000001t', 0xFFFE) )
# 10 End of block
vlc.append( ('10t', 0xFFFF) )

# build huffman tree
for code in vlc:
	addCode(code[0], code[1])

# output C structure
print 'static const uint16_t kAcHuff_EscapeCode = 0xFFFE;'
print 'static const uint16_t kAcHuff_EndOfBlock = 0xFFFF;'
print 'struct AcHuff {'
print '\t int16_t left;'
print '\t int16_t right;'
print '\t uint16_t value;'
print '};'
print 'static const AcHuff _acHuffTree[] = {'
for node in NODES:
	dumpNode(node)
print '};'
