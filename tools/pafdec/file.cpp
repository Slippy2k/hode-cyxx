
#include "file.h"


struct File_impl {
	bool _ioErr;
	File_impl() : _ioErr(false) {}
	virtual bool open(const char *path, const char *mode) = 0;
	virtual void close() = 0;
	virtual uint32 size() = 0;
	virtual void seek(int32 off, int whence) = 0;
	virtual void read(void *ptr, uint32 len) = 0;
	virtual void write(const void *ptr, uint32 len) = 0;
};

struct stdFile : File_impl {
	FILE *_fp;
	stdFile() : _fp(0) {}
	bool open(const char *path, const char *mode) {
		_ioErr = false;
		_fp = fopen(path, mode);
		return (_fp != 0);
	}
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = 0;
		}
	}
	uint32 size() {
		uint32 sz = 0;
		if (_fp) {
			int pos = ftell(_fp);
			fseek(_fp, 0, SEEK_END);
			sz = ftell(_fp);
			fseek(_fp, pos, SEEK_SET);
		}
		return sz;
	}
	void seek(int32 off, int whence) {
		if (_fp) {
			fseek(_fp, off, whence);
		}
	}
	void read(void *ptr, uint32 len) {
		if (_fp) {
			uint32 r = fread(ptr, 1, len, _fp);
			if (r != len) {
				_ioErr = true;
			}
		}
	}
	void write(const void *ptr, uint32 len) {
		if (_fp) {
			uint32 r = fwrite(ptr, 1, len, _fp);
			if (r != len) {
				_ioErr = true;
			}
		}
	}
};

File::File() {
	_impl = new stdFile;
}

File::~File() {
	_impl->close();
	delete _impl;
}

bool File::open(const char *filename, const char *mode) {
	_impl->close();
	return _impl->open(filename, mode);
}

void File::close() {
	_impl->close();
}

bool File::ioErr() const {
	return _impl->_ioErr;
}

uint32 File::size() {
	return _impl->size();
}

void File::seek(int32 off, int whence) {
	_impl->seek(off, whence);
}

void File::read(void *ptr, uint32 len) {
	_impl->read(ptr, len);
}

uint8 File::readByte() {
	uint8 b;
	read(&b, 1);
	return b;
}

uint16 File::readUint16LE() {
	uint8 lo = readByte();
	uint8 hi = readByte();
	return (hi << 8) | lo;
}

uint32 File::readUint32LE() {
	uint16 lo = readUint16LE();
	uint16 hi = readUint16LE();
	return (hi << 16) | lo;
}

uint16 File::readUint16BE() {
	uint8 hi = readByte();
	uint8 lo = readByte();
	return (hi << 8) | lo;
}

uint32 File::readUint32BE() {
	uint16 hi = readUint16BE();
	uint16 lo = readUint16BE();
	return (hi << 16) | lo;
}

void File::write(const void *ptr, uint32 len) {
	_impl->write(ptr, len);
}

void File::writeByte(uint8 b) {
	write(&b, 1);
}

void File::writeUint16LE(uint16 n) {
	writeByte(n & 0xFF);
	writeByte(n >> 8);
}

void File::writeUint32LE(uint32 n) {
	writeUint16LE(n & 0xFFFF);
	writeUint16LE(n >> 16);
}

void File::writeUint16BE(uint16 n) {
	writeByte(n >> 8);
	writeByte(n & 0xFF);
}

void File::writeUint32BE(uint32 n) {
	writeUint16BE(n >> 16);
	writeUint16BE(n & 0xFFFF);
}
