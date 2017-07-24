
#include <stdio.h>
#include <png.h>

#include "writer.h"

class PngFileWriter : public ImageWriter {
public:

	virtual ~PngFileWriter() {}
	virtual bool Open(const char *filename, int width, int height);
	virtual void Close();
	virtual void Write(const uint8 *src, int pitch, const uint8 *palette);

private:

	int m_width;
	int m_height;
	FILE *m_fp;
};

bool PngFileWriter::Open(const char *filename, int width, int height) {

	m_width = width;
	m_height = height;

	char filepath[512];
	sprintf(filepath, "%s.png", filename);
	m_fp = fopen(filepath, "wb");
	return m_fp != NULL;
}

void PngFileWriter::Close() {

	if (m_fp) {
		fclose(m_fp);
		m_fp = 0;
	}
}

void PngFileWriter::Write(const uint8 *src, int pitch, const uint8 *palette) {

	if (m_fp) {

		png_struct *png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		if (!png)
			return;

		png_info *png_info = png_create_info_struct(png);
		if (!png_info)
			return;

		png_init_io(png, m_fp);

		png_set_IHDR(png, png_info, m_width, m_height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		png_color *png_pal = (png_color *)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
		for (int i = 0; i != 256; ++i) {
			png_pal[i].red = (palette[0] << 2) | (palette[0] >> 4);
			png_pal[i].green = (palette[1] << 2) | (palette[1] >> 4);
			png_pal[i].blue = (palette[2] << 2) | (palette[2] >> 4);
			palette += 3;
		}
		png_set_PLTE(png, png_info, png_pal, PNG_MAX_PALETTE_LENGTH);

		png_write_info(png, png_info);
		png_set_flush(png, 10);

		png_byte *png_scanlines[256];
		assert(m_height <= 256);
		for (int i = 0; i < m_height; i++) {
			png_scanlines[i] = (png_byte *)src;
			src += pitch;
		}

		png_write_image(png, png_scanlines);

		png_write_end(png, png_info);

		png_free(png, png_pal);

		png_destroy_write_struct(&png, &png_info);
	}
}

ImageWriter *createPngImageWriter() {
	return new PngFileWriter;
}
