
#include <assert.h>
#include <stdio.h>
#include <sys/param.h>
#include "fileio.h"
#include "screenshot.h"

static void TO_LE16(uint8_t *dst, uint16_t value) {
	for (int i = 0; i < 2; ++i) {
		dst[i] = value & 255;
		value >>= 8;
	}
}

#define kTgaImageTypeUncompressedTrueColor 2
#define kTgaImageTypeRunLengthEncodedTrueColor 10
#define kTgaDirectionTop (1 << 5)

static const int TGA_HEADER_SIZE = 18;

void saveTGA(const char *filename, const uint8_t *rgb, int w, int h) {

	static const uint8_t kImageType = kTgaImageTypeRunLengthEncodedTrueColor;
	uint8_t buffer[TGA_HEADER_SIZE];
	buffer[0]            = 0; // ID Length
	buffer[1]            = 0; // ColorMap Type
	buffer[2]            = kImageType;
	TO_LE16(buffer +  3,   0); // ColorMap Start
	TO_LE16(buffer +  5,   0); // ColorMap Length
	buffer[7]            = 0;  // ColorMap Bits
	TO_LE16(buffer +  8,   0); // X-origin
	TO_LE16(buffer + 10,   0); // Y-origin
	TO_LE16(buffer + 12,   w); // Image Width
	TO_LE16(buffer + 14,   h); // Image Height
	buffer[16]           = 24; // Pixel Depth
	buffer[17]           = kTgaDirectionTop;  // Descriptor

	FILE *fp = fopen(filename, "wb");
	if (fp) {
		fwrite(buffer, sizeof(buffer), 1, fp);
		if (kImageType == kTgaImageTypeUncompressedTrueColor) {
			for (int i = 0; i < w * h; ++i) {
				fputc(rgb[2], fp);
				fputc(rgb[1], fp);
				fputc(rgb[0], fp);
				rgb += 3;
			}
		} else {
			assert(kImageType == kTgaImageTypeRunLengthEncodedTrueColor);
			int prevColor = rgb[0] + (rgb[1] << 8) + (rgb[2] << 16); rgb += 3;
			int count = 0;
			for (int i = 1; i < w * h; ++i) {
				int color = rgb[0] + (rgb[1] << 8) + (rgb[2] << 16); rgb += 3;
				if (prevColor == color && count < 127) {
					++count;
					continue;
				}
				fputc(count | 0x80, fp);
				fputc((prevColor >> 16) & 255, fp);
				fputc((prevColor >>  8) & 255, fp);
				fputc( prevColor        & 255, fp);
				count = 0;
				prevColor = color;
			}
			if (count != 0) {
				fputc(count | 0x80, fp);
				fputc((prevColor >> 16) & 255, fp);
				fputc((prevColor >>  8) & 255, fp);
				fputc( prevColor        & 255, fp);
			}
		}
		fclose(fp);
	}
}

static const uint16_t TAG_BM = 0x4D42;

void saveBMP(const char *filename, const uint8_t *bits, const uint8_t *pal, int w, int h) {
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		int alignWidth = (w + 3) & ~3;
		int imageSize = alignWidth * h;

		// Write file header
		fwriteUint16LE(fp, TAG_BM);
		fwriteUint32LE(fp, 14 + 40 + 4 * 256 + imageSize);
		fwriteUint16LE(fp, 0); // reserved1
		fwriteUint16LE(fp, 0); // reserved2
		fwriteUint32LE(fp, 14 + 40 + 4 * 256);

		// Write info header
		fwriteUint32LE(fp, 40);
		fwriteUint32LE(fp, w);
		fwriteUint32LE(fp, h);
		fwriteUint16LE(fp, 1); // planes
		fwriteUint16LE(fp, 8); // bit_count
		fwriteUint32LE(fp, 0); // compression
		fwriteUint32LE(fp, imageSize); // size_image
		fwriteUint32LE(fp, 0); // x_pels_per_meter
		fwriteUint32LE(fp, 0); // y_pels_per_meter
		fwriteUint32LE(fp, 0); // num_colors_used
		fwriteUint32LE(fp, 0); // num_colors_important

		// Write palette data
		for (int i = 0; i < 256; ++i) {
			fputc(pal[2], fp);
			fputc(pal[1], fp);
			fputc(pal[0], fp);
			fputc(0, fp);
			pal += 3;
		}

		// Write bitmap data
		const int pitch = w;
		bits += h * pitch;
		for (int i = 0; i < h; ++i) {
			bits -= pitch;
			fwrite(bits, w, 1, fp);
			int pad = alignWidth - w;
			while (pad--) {
				fputc(0, fp);
			}
		}

		fclose(fp);
	}
}

static void tiffWriteTag(FILE *fp, uint16_t tag, uint16_t type, uint32_t count, uint32_t value) {
	fwriteUint16LE(fp, tag);
	fwriteUint16LE(fp, type);
	fwriteUint32LE(fp, count);
	fwriteUint32LE(fp, value);
}

static const int kTiffType_Short = 3;
static const int kTiffType_Long  = 4;

static const int kTiffTag_ImageWidth = 0x100;
static const int kTiffTag_ImageLength = 0x101;
static const int kTiffTag_BitsPerSample = 0x102;
static const int kTiffTag_Compression = 0x103;
static const int kTiffTag_PhotometricInterpretation = 0x106;
static const int kTiffTag_StripOffsets = 0x111;
static const int kTiffTag_SamplesPerPixel = 0x115;
static const int kTiffTag_RowsPerStrip = 0x116;
static const int kTiffTag_StripByteCounts = 0x117;
static const int kTiffTag_ColorMap = 0x140;

void saveLZW(const char *filename, const uint8_t *bits, int len, const uint8_t *pal, int w, int h) {
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		fwriteUint16LE(fp, 0x4949);
		fwriteUint16LE(fp, 42);
		// IFD offset
		int offset = 8 + len + 256 * 3 * sizeof(uint16_t);
		fwriteUint32LE(fp, offset);

		// lzw encoded data
		fwrite(bits, 1, len, fp);
		// color map
		for (int j = 0; j < 3; ++j) {
			for (int i = 0; i < 256; ++i) {
				const uint16_t color = (pal[i * 3 + j] << 8) | pal[i * 3 + j];
				fwriteUint16LE(fp, color);
			}
		}

		static const int kTagsCount = 10;
		fwriteUint16LE(fp, kTagsCount);

		tiffWriteTag(fp, kTiffTag_ImageWidth, kTiffType_Short, 1, w);
		tiffWriteTag(fp, kTiffTag_ImageLength, kTiffType_Short, 1, h);
		tiffWriteTag(fp, kTiffTag_BitsPerSample, kTiffType_Short, 1, 8);
		tiffWriteTag(fp, kTiffTag_Compression, kTiffType_Short, 1, 5); // LZW
		tiffWriteTag(fp, kTiffTag_PhotometricInterpretation, kTiffType_Short, 1, 3);
		tiffWriteTag(fp, kTiffTag_SamplesPerPixel, kTiffType_Short, 1, 1);
		tiffWriteTag(fp, kTiffTag_RowsPerStrip, kTiffType_Short, 1, h);
		tiffWriteTag(fp, kTiffTag_StripOffsets, kTiffType_Long, 1, 8);
		tiffWriteTag(fp, kTiffTag_StripByteCounts, kTiffType_Long, 1, len);
		tiffWriteTag(fp, kTiffTag_ColorMap, kTiffType_Short, 256 * 3, 8 + len);

		// no other IFD
		fwriteUint32LE(fp, 0);

		fclose(fp);
	}
}

#ifdef _WIN32
void savePSX(const char *filename, const uint8_t *src, int len, int w, int h) {
	char filename2[MAXPATHLEN];
	strcpy(filename2, filename);
	char *ext = strrchr(filename2, '.');
	if (ext && strcmp(ext, ".jpg") == 0) {
		strcpy(ext + 1, "bss");
		fioDumpData(filename2, src, len);
	}
}
#else
extern "C" {
	#include <jpeglib.h>
	#include <libavcodec/avcodec.h>
}

void savePSX(const char *filename, const uint8_t *src, int len, int w, int h) {
	static bool codec_inited = false;

	if (!codec_inited) {
		avcodec_register_all();
		codec_inited = true;
	}

	fprintf(stdout, "MDEC len %d, VLC_ID 0x%x\n", READ_LE_UINT16(src), READ_LE_UINT16(src + 2));
	fprintf(stdout, "qscale %d version %d\n", READ_LE_UINT16(src + 4), READ_LE_UINT16(src + 6));

	const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MDEC);
	AVCodecContext *ctx = avcodec_alloc_context3(codec);
	ctx->width  = w;
	ctx->height = h;
	avcodec_open2(ctx, codec, 0);

	AVPacket pkt;
	av_new_packet(&pkt, len);
	memcpy(pkt.data, src, len);

	AVFrame *frame = av_frame_alloc();
	int hasFrame = 0;
	const int ret = avcodec_decode_video2(ctx, frame, &hasFrame, &pkt);
	if (ret < 0) {
		fprintf(stderr, "avcodec_decode_video2 ret %d\n", ret);
	} else {

		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);

		FILE *fp = fopen(filename, "wb");
		if (fp) {
			jpeg_stdio_dest(&cinfo, fp);

			cinfo.image_width = w;
			cinfo.image_height = h;
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_YCbCr;

			jpeg_set_defaults(&cinfo);
			cinfo.raw_data_in = TRUE;

			cinfo.comp_info[0].h_samp_factor = 2;
			cinfo.comp_info[0].v_samp_factor = 2;
			cinfo.comp_info[0].dc_tbl_no = 0;
			cinfo.comp_info[0].ac_tbl_no = 0;
			cinfo.comp_info[0].quant_tbl_no = 0;

			cinfo.comp_info[1].h_samp_factor = 1;
			cinfo.comp_info[1].v_samp_factor = 1;
			cinfo.comp_info[1].dc_tbl_no = 1;
			cinfo.comp_info[1].ac_tbl_no = 1;
			cinfo.comp_info[1].quant_tbl_no = 1;

			cinfo.comp_info[2].h_samp_factor = 1;
			cinfo.comp_info[2].v_samp_factor = 1;
			cinfo.comp_info[2].dc_tbl_no = 1;
			cinfo.comp_info[2].ac_tbl_no = 1;
			cinfo.comp_info[2].quant_tbl_no = 1;

			jpeg_set_quality(&cinfo, 100, TRUE);
			cinfo.optimize_coding = TRUE;

			JSAMPROW y[16], cb[16], cr[16];
			JSAMPARRAY p[3] = { y, cr, cb };

			jpeg_start_compress(&cinfo, TRUE);

			for (unsigned int j = 0; j < cinfo.image_height; j += 16) {
				int offset = j;
				for (unsigned int i = 0; i < 16; i++) {
					y[i]       = frame->data[0] + frame->linesize[0] *  offset;
					cr[i >> 1] = frame->data[1] + frame->linesize[1] * (offset >> 1);
					cb[i >> 1] = frame->data[2] + frame->linesize[2] * (offset >> 1);
					++offset;
				}
				jpeg_write_raw_data(&cinfo, p, 16);
			}

			jpeg_finish_compress(&cinfo);
			jpeg_destroy_compress(&cinfo);

			fclose(fp);
		}
	}
	avcodec_free_context(&ctx);
	av_frame_free(&frame);
}
#endif // _WIN32
