
#include "file.h"
#include "writer.h"

static const uint16 TAG_BM = 0x4D42;

class BmpFileWriter : public ImageWriter {
public:

	virtual ~BmpFileWriter() {}
	virtual bool Open(const char *filename, int width, int height);
	virtual void Close();
	virtual void Write(const uint8 *src, int pitch, const uint8 *palette);

private:

	int m_width;
	int m_height;
	int m_alignWidth;
	bool m_isOpened;
	File m_file;
};

bool BmpFileWriter::Open(const char *filename, int width, int height) {

	m_width = width;
	m_height = height;
	m_alignWidth = (width + 3) & ~3;

	char filepath[512];
	sprintf(filepath, "%s.bmp", filename);
	m_isOpened = m_file.open(filepath, "wb");
	return m_isOpened;
}

void BmpFileWriter::Close() {

	if (m_isOpened) {
		m_isOpened = false;
		m_file.close();
	}
}

void BmpFileWriter::Write(const uint8 *src, int pitch, const uint8 *palette) {

	if (m_isOpened) {
		int imageSize = m_alignWidth * m_height;

		// Write file header
		m_file.writeUint16LE(TAG_BM);
		m_file.writeUint32LE(14 + 40 + 4 * 256 + imageSize);
		m_file.writeUint16LE(0); // reserved1
		m_file.writeUint16LE(0); // reserved2
		m_file.writeUint32LE(14 + 40 + 4 * 256);

		// Write info header
		m_file.writeUint32LE(40);
		m_file.writeUint32LE(m_width);
		m_file.writeUint32LE(m_height);
		m_file.writeUint16LE(1); // planes
		m_file.writeUint16LE(8); // bit_count
		m_file.writeUint32LE(0); // compression
		m_file.writeUint32LE(imageSize); // size_image
		m_file.writeUint32LE(0); // x_pels_per_meter
		m_file.writeUint32LE(0); // y_pels_per_meter
		m_file.writeUint32LE(0); // num_colors_used
		m_file.writeUint32LE(0); // num_colors_important

		// Write palette data
		for (int i = 0; i < 256; ++i) {
			m_file.writeByte((palette[2] << 2) | (palette[2] >> 4));
			m_file.writeByte((palette[1] << 2) | (palette[1] >> 4));
			m_file.writeByte((palette[0] << 2) | (palette[0] >> 4));
			m_file.writeByte(0);
			palette += 3;
		}

		// Write bitmap data
		src += m_height * pitch;
		for (int i = 0; i < m_height; ++i) {
			src -= pitch;
			m_file.write(src, m_width);
			int pad = m_alignWidth - m_width;
			while (pad--) {
				m_file.writeByte(0);
			}
		}
	}
}

ImageWriter *createBmpImageWriter() {
	return new BmpFileWriter;
}
