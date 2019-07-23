
#ifndef WRITER_H__
#define WRITER_H__

#include "intern.h"

struct ImageWriter {
	virtual ~ImageWriter() {}
	virtual bool Open(const char *filename, int width, int height) = 0;
	virtual void Close() = 0;
	virtual void Write(const uint8_t *src, int pitch, const uint8_t *palette) = 0;
};

struct SoundWriter {
	virtual ~SoundWriter() {}
	virtual bool Open(const char *filename, int sampleRate, int bitsPerSample, int numChannels, bool isLittleEndian) = 0;
	virtual void Close() = 0;
	virtual void Write(const uint8_t *src, int len) = 0;
};

ImageWriter *createPngImageWriter();
ImageWriter *createBmpImageWriter();
SoundWriter *createWavSoundWriter();

#endif // WRITER_H__
