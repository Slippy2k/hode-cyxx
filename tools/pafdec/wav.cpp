
#include "file.h"
#include "writer.h"

static const uint32 TAG_RIFF = 0x52494646;
static const uint32 TAG_WAVE = 0x57415645;
static const uint32 TAG_fmt  = 0x666d7420;
static const uint32 TAG_data = 0x64617461;

class WavFileWriter : public SoundWriter {
public:

	virtual ~WavFileWriter() {}
	virtual bool Open(const char *filename, int sampleRate, int bitsPerSample, int numChannels, bool isLittleEndian);
	virtual void Close();
	virtual void Write(const uint8 *src, int len);

private:

	void WriteHeader();

	int m_sampleRate;
	int m_bitsPerSample;
	int m_numChannels;
	bool m_isLittleEndian;

	int m_fmtChunkSize;
	int m_dataChunkSize;

	bool m_isOpened;
	File m_file;
};

bool WavFileWriter::Open(const char *filename, int sampleRate, int bitsPerSample, int numChannels, bool isLittleEndian) {

	m_sampleRate = sampleRate;
	m_bitsPerSample = bitsPerSample;
	m_numChannels = numChannels;
	m_isLittleEndian = isLittleEndian;

	m_fmtChunkSize = 0;
	m_dataChunkSize = 0;

	m_isOpened = false;
	char filepath[512];
	sprintf(filepath, "%s.wav", filename);
	if (m_file.open(filepath, "wb")) {
		WriteHeader();
		m_isOpened = true;
	}
	return m_isOpened;
}

void WavFileWriter::Close() {

	if (m_isOpened) {
		m_isOpened = false;

		// Update RIFF chunk size
		m_file.seek(4);
		m_file.writeUint32LE(4 + 8 + m_fmtChunkSize + 8 + m_dataChunkSize);

		// Update data chunk size
		m_file.seek(12 + 8 + m_fmtChunkSize + 4);
		m_file.writeUint32LE(m_dataChunkSize);

		m_file.close();
	}
}

void WavFileWriter::Write(const uint8 *src, int len) {
	assert(m_bitsPerSample == 8 || m_bitsPerSample == 16);

	if (m_bitsPerSample == 16) {
		assert((len & 1) == 0);
		for (int i = 0; i < len; i += 2) {
			int16 pcm = *(int16 *)src; src += 2;
			if (m_isLittleEndian) {
				m_file.writeUint16LE(pcm);
			} else {
				m_file.writeUint16BE(pcm);
			}
		}
	} else {
		m_file.write(src, len);
	}

	m_dataChunkSize += len;
}

void WavFileWriter::WriteHeader() {
	static const uint32 invalidChunkSize = 0x12345678;

	// Write RIFF chunk descriptor
	m_file.writeUint32BE(TAG_RIFF);
	m_file.writeUint32LE(invalidChunkSize); // chunkSize
	m_file.writeUint32BE(TAG_WAVE); // format

	// Write fmt chunk
	m_file.writeUint32BE(TAG_fmt);
	m_file.writeUint32LE(16);
	m_file.writeUint16LE(1); // AudioFormat PCM
	m_file.writeUint16LE(m_numChannels);
	m_file.writeUint32LE(m_sampleRate);
	m_file.writeUint32LE(m_sampleRate * m_numChannels * m_bitsPerSample / 8); // ByteRate
	m_file.writeUint16LE(m_numChannels * m_bitsPerSample / 8); // BlockAlign
	m_file.writeUint16LE(m_bitsPerSample);
	m_fmtChunkSize = 16;

	// Write data chunk
	m_file.writeUint32BE(TAG_data);
	m_file.writeUint32LE(invalidChunkSize);
	m_dataChunkSize = 0;
}

SoundWriter *createWavSoundWriter() {
	return new WavFileWriter;
}
