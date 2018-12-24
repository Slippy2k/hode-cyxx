
#include <stdio.h>
#include <png.h>
#include "file.h"

void raw2png(FILE *fp, const uint8_t *src, int width, int height, const uint8_t *palette, int raw2png_6bits_color) {
	png_struct *png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png)
		return;

	png_info *png_info = png_create_info_struct(png);
	if (!png_info)
		return;

	png_init_io(png, fp);

	png_set_IHDR(png, png_info, width, height, 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_color *png_pal = (png_color *)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
	for (int i = 0; i != 256; ++i) {
		if (raw2png_6bits_color) {
			png_pal[i].red = (palette[0] << 2) | (palette[0] >> 4);
			png_pal[i].green = (palette[1] << 2) | (palette[1] >> 4);
			png_pal[i].blue = (palette[2] << 2) | (palette[2] >> 4);
		} else {
			png_pal[i].red = palette[0];
			png_pal[i].green = palette[1];
			png_pal[i].blue = palette[2];
		}
		palette += 3;
	}
	png_set_PLTE(png, png_info, png_pal, PNG_MAX_PALETTE_LENGTH);

	png_write_info(png, png_info);
	png_set_flush(png, 10);

	png_byte *png_scanlines[1024];
	assert(height <= 1024);
	for (int i = 0; i < height; i++) {
		png_scanlines[i] = (png_byte *)src;
		src += width;
	}

	png_write_image(png, png_scanlines);

	png_write_end(png, png_info);

	png_free(png, png_pal);

	png_destroy_write_struct(&png, &png_info);
}
