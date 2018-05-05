
//
// Heart Of Darkness PAF Decoder
// Copyright (c) 2007 Gregory Montoir
//

#include "paf_decoder.h"
#include "writer.h"


bool PafDecoder::Open(const char *filename, int videoNum) {
	m_opened = m_file.open(filename, "rb");
	if (m_opened) {
		if (videoNum >= 0 && videoNum < kMaxVideosCount) {
			SeekToVideo(videoNum);
			m_videoNum = videoNum;
		} else {
			m_videoOffset = 0;
			m_videoNum = 99;
		}
		if (!ReadPafHeader()) {
			printf("Unable to read .PAF file header");
			return false;
		}
		for (int i = 0; i < 4; ++i) {
			m_videoPages[i] = (uint8_t *)calloc(1, kVideoWidth * 256);
		}
		m_demuxVideoFrameBlocks = (uint8_t *)calloc(m_pafHdr.maxVideoFrameBlocksCount, m_pafHdr.readBufferSize);
		m_demuxAudioFrameBlocks = (uint8_t *)calloc(m_pafHdr.maxAudioFrameBlocksCount, m_pafHdr.readBufferSize);
	}
	return m_opened;
}

void PafDecoder::Close() {
	if (m_opened) {
		m_opened = false;

		m_file.close();

		free(m_pafHdr.frameBlocksCountTable);
		free(m_pafHdr.framesOffsetTable);
		free(m_pafHdr.frameBlocksOffsetTable);

		for (int i = 0; i < 4; ++i) {
			free(m_videoPages[i]);
		}
		free(m_demuxVideoFrameBlocks);
		free(m_demuxAudioFrameBlocks);
	}
}

void PafDecoder::Decode() {
	m_file.seek(m_videoOffset + m_pafHdr.startOffset);

	memset(m_paletteBuffer, 0, sizeof(m_paletteBuffer));
	m_currentVideoPage = 0;

	m_imageWriter = createPngImageWriter();
	m_soundWriter = createWavSoundWriter();

	char buf[512];
	sprintf(buf, "s_%02d", m_videoNum);
	m_soundWriter->Open(buf, 22050, 16, 2, true);

	int currentFrameBlock = 0;
	for (uint32_t i = 0; i < m_pafHdr.framesCount; ++i) {

		printf("Decoding frame %d/%d... ", i, m_pafHdr.framesCount);

		// read buffering blocks
		uint32_t blocksCountForFrame = (i == 0) ? m_pafHdr.preloadFrameBlocksCount : m_pafHdr.frameBlocksCountTable[i - 1];
		while (blocksCountForFrame != 0) {
			m_file.read(m_bufferBlock, m_pafHdr.readBufferSize);
			uint32_t dstOffset = m_pafHdr.frameBlocksOffsetTable[currentFrameBlock] & ~(1 << 31);
			if (m_pafHdr.frameBlocksOffsetTable[currentFrameBlock] & (1 << 31)) {
				assert(dstOffset + m_pafHdr.readBufferSize <= m_pafHdr.maxAudioFrameBlocksCount * m_pafHdr.readBufferSize);
				memcpy(m_demuxAudioFrameBlocks + dstOffset, m_bufferBlock, m_pafHdr.readBufferSize);
				DecodeAudioFrame(m_demuxAudioFrameBlocks, dstOffset);
			} else {
				assert(dstOffset + m_pafHdr.readBufferSize <= m_pafHdr.maxVideoFrameBlocksCount * m_pafHdr.readBufferSize);
				memcpy(m_demuxVideoFrameBlocks + dstOffset, m_bufferBlock, m_pafHdr.readBufferSize);
			}
			++currentFrameBlock;
			--blocksCountForFrame;
		}

		// decode video data
		DecodeVideoFrame(m_demuxVideoFrameBlocks + m_pafHdr.framesOffsetTable[i]);

		// write image
		sprintf(buf, "i_%02d_%04d", m_videoNum, i);
		if (m_imageWriter->Open(buf, kVideoWidth, kVideoHeight)) {
			m_imageWriter->Write(m_videoPages[m_currentVideoPage], kVideoWidth, m_paletteBuffer);
			m_imageWriter->Close();
		}

		printf("Ok\n");

		// set next decoding video page
		++m_currentVideoPage;
		m_currentVideoPage &= 3;
	}

	// flush sound data decoding
	m_soundWriter->Close();

	delete m_imageWriter;
	m_imageWriter = 0;
	delete m_soundWriter;
	m_soundWriter = 0;
}

void PafDecoder::SeekToVideo(int videoNum) {
	assert(videoNum >= 0 && videoNum < kMaxVideosCount);
	m_file.seek(videoNum * 4);
	m_videoOffset = m_file.readUint32LE();
	m_file.seek(m_videoOffset);
}

void PafDecoder::AlignReadHeaderTable(uint32_t *dst, int count) {
	assert((count & 3) == 0);
	for (int i = 0; i < count / 4; ++i) {
		dst[i] = m_file.readUint32LE();
	}
	count &= 0x7FF;
	if (count != 0) {
		m_file.seek(0x800 - count, SEEK_CUR);
	}
}

bool PafDecoder::ReadPafHeader() {
	static const char *headerSignature = "Packed Animation File V1.0\n(c) 1992-96 Amazing Studio\n";

	memset(&m_pafHdr, 0, sizeof(m_pafHdr));

	m_file.read(m_bufferBlock, sizeof(m_bufferBlock));

	if (memcmp(m_bufferBlock, headerSignature, strlen(headerSignature)) != 0) {
		return false;
	}

	m_pafHdr.startOffset = READ_LE_UINT32(m_bufferBlock + 0xA4);
	m_pafHdr.preloadFrameBlocksCount = READ_LE_UINT32(m_bufferBlock + 0x9C);
	m_pafHdr.readBufferSize = READ_LE_UINT32(m_bufferBlock + 0x98);
	m_pafHdr.framesCount = READ_LE_UINT32(m_bufferBlock + 0x84);
	m_pafHdr.maxVideoFrameBlocksCount = READ_LE_UINT32(m_bufferBlock + 0xA8);
	m_pafHdr.maxAudioFrameBlocksCount = READ_LE_UINT32(m_bufferBlock + 0xAC);
	m_pafHdr.frameBlocksCount = READ_LE_UINT32(m_bufferBlock + 0xA0);

	assert(m_pafHdr.readBufferSize <= sizeof(m_bufferBlock));

	m_pafHdr.frameBlocksCountTable = (uint32_t *)malloc(m_pafHdr.framesCount * sizeof(uint32_t));
	AlignReadHeaderTable(m_pafHdr.frameBlocksCountTable, m_pafHdr.framesCount * sizeof(uint32_t));

	m_pafHdr.framesOffsetTable = (uint32_t *)malloc(m_pafHdr.framesCount * sizeof(uint32_t));
	AlignReadHeaderTable(m_pafHdr.framesOffsetTable, m_pafHdr.framesCount * sizeof(uint32_t));

	m_pafHdr.frameBlocksOffsetTable = (uint32_t *)malloc(m_pafHdr.frameBlocksCount * sizeof(uint32_t));
	AlignReadHeaderTable(m_pafHdr.frameBlocksOffsetTable, m_pafHdr.frameBlocksCount * sizeof(uint32_t));

	return true;
}

void PafDecoder::DecodeAudioFrame(const uint8_t *src, uint32_t offset) {
	assert(m_pafHdr.maxAudioFrameBlocksCount > 2);
	uint32_t endOffset = (m_pafHdr.maxAudioFrameBlocksCount - 2) * m_pafHdr.readBufferSize;
	if (offset == endOffset) {
		int soundBuffersCount = (endOffset + m_pafHdr.readBufferSize) / kSoundBufferSize;
		while (soundBuffersCount--) {
			DecodeAudioFrame2205(src);
			src += kSoundBufferSize;
		}
	}
}

void PafDecoder::DecodeAudioFrame2205(const uint8_t *src) {
	int count = 2205;
	const uint8_t *t = src;
	src += 256 * sizeof(uint16_t);
	while (count--) {
		for (int channel = 0; channel < 2; ++channel) {
			int index = *src++;
			int16_t pcm = (int16_t)READ_LE_UINT16(t + index * 2);
			m_soundWriter->Write((const uint8_t *)&pcm, 2);
		}
	}
}

void PafDecoder::DecodeVideoFrame(const uint8_t *src) {
	const uint8_t *base = src;
	uint8_t code = *src++;
	if (code & 0x20) {
		for (int i = 0; i < 4; ++i) {
			memset(m_videoPages[i], 0, kVideoWidth * 256);
		}
		memset(m_paletteBuffer, 0, sizeof(m_paletteBuffer));
		m_currentVideoPage = 0;
	}
	if (code & 0x40) {
		const int index = src[0];
		const int count = (src[1] + 1) * 3;
		assert(index * 3 + count <= 768);
		src += 2;
		memcpy(&m_paletteBuffer[index * 3], src, count);
		src += count;
	}
	switch (code & 0xF) {
	case 0:
		DecodeVideoFrameOp0(base, src, code);
		break;
	case 1:
		DecodeVideoFrameOp1(src);
		break;
	case 2:
		DecodeVideoFrameOp2(src);
		break;
	case 4:
		DecodeVideoFrameOp4(src);
		break;
	default:
		assert(0);
	}
}

static void pafCopy4x4h(uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 4; ++i) {
		memcpy(dst, src, 4);
		src += 4;
		dst += 256;
	}
}

static void pafCopy4x4v(uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 4; ++i) {
		memcpy(dst, src, 4);
		src += 256;
		dst += 256;
	}
}

static void pafCopySrcMask(uint8_t mask, uint8_t *dst, const uint8_t *src) {
	for (int i = 0; i < 4; ++i) {
		if (mask & (1 << (3 - i))) {
			dst[i] = src[i];
		}
	}
}

static void pafCopyColorMask(uint8_t mask, uint8_t *dst, uint8_t color) {
	for (int i = 0; i < 4; ++i) {
		if (mask & (1 << (3 - i))) {
			dst[i] = color;
		}
	}
}

static const uint8_t updateBlockSequences[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	2, 0, 0, 0, 0, 0, 0, 0,
	5, 7, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0,
	6, 0, 0, 0, 0, 0, 0, 0,
	5, 7, 5, 7, 0, 0, 0, 0,
	5, 7, 5, 0, 0, 0, 0, 0,
	5, 7, 6, 0, 0, 0, 0, 0,
	5, 5, 0, 0, 0, 0, 0, 0,
	3, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 0, 0, 0, 0, 0, 0,
	2, 4, 0, 0, 0, 0, 0, 0,
	2, 4, 5, 7, 0, 0, 0, 0,
	2, 4, 5, 0, 0, 0, 0, 0,
	2, 4, 6, 0, 0, 0, 0, 0,
	2, 4, 5, 7, 5, 7, 0, 0
};

uint8_t *PafDecoder::GetVideoPageOffset(uint8_t a, uint8_t b) {
	int x = b & 0x7F;
	int y = ((a & 0x3F) << 1) | ((b >> 7) & 1);
	int page = (a & 0xC0) >> 6;
	return m_videoPages[page] + y * 2 * 256 + x * 2;
}

void PafDecoder::DecodeVideoFrameOp0(const uint8_t *base, const uint8_t *src, uint8_t code) {
	int count = *src++;
	if (count != 0) {
		if ((code & 0x10) != 0) {
			int align = src - base;
			align &= 3;
			if (align != 0) {
				src += 4 - align;
			}
		}
		do {
			uint8_t *dst = GetVideoPageOffset(src[0], src[1]);
			uint32_t offset = (src[1] & 0x7F) * 2;
			uint32_t end = READ_LE_UINT16(src + 2); src += 4;
			end += offset;
			do {
				++offset;
				pafCopy4x4h(dst, src);
				src += 16;
				if ((offset & 0x3F) == 0) {
					dst += 256 * 3;
				}
				dst += 4;
			} while (offset < end);
		} while (--count != 0);
	}

	uint8_t *dst = m_videoPages[m_currentVideoPage];
	count = 0;
	do {
		const uint8_t *src2 = GetVideoPageOffset(src[0], src[1]); src += 2;
		pafCopy4x4v(dst, src2);
		++count;
		if ((count & 0x3F) == 0) {
			dst += 256 * 3;
		}
		dst += 4;
	} while (count < 256 * 192 / 16);

	uint32_t opcodesSize = READ_LE_UINT16(src); src += 4;

	const uint8_t *opcodes;
	const uint8_t *opcodesData = src;
	src += opcodesSize;

	uint8_t mask = 0;
	uint8_t color = 0;
	uint32_t offset = 0;
	const uint8_t *src2 = 0;

	dst = m_videoPages[m_currentVideoPage];

	uint8_t h = 192 / 4;
	do {
		uint8_t w = 256 / 4;
		do {
			if ((w & 1) == 0) {
				opcodes = &updateBlockSequences[(*opcodesData >> 4) * 8];
			} else {
				opcodes = &updateBlockSequences[(*opcodesData & 15) * 8];
				++opcodesData;
			}
			while (*opcodes) {
				offset = 256 * 2;
				uint8_t code = *opcodes++;
				switch (code) {
				case 2:
					offset = 0;
				case 3:
					color = *src++;
				case 4:
					mask = *src++;
					pafCopyColorMask(mask >> 4, dst + offset, color);
					offset += 256;
					pafCopyColorMask(mask & 15, dst + offset, color);
					break;
				case 5:
					offset = 0;
				case 6:
					src2 = GetVideoPageOffset(src[0], src[1]); src += 2;
				case 7:
					mask = *src++;
					pafCopySrcMask(mask >> 4, dst + offset, src2 + offset);
					offset += 256;
					pafCopySrcMask(mask & 15, dst + offset, src2 + offset);
					break;
				}
			}
			dst += 4;
		} while (--w != 0);
		dst += 256 * 3;
	} while (--h != 0);
}

void PafDecoder::DecodeVideoFrameOp1(const uint8_t *src) {
	uint8_t *dst = m_videoPages[m_currentVideoPage];
	memcpy(dst, src + 2, kVideoWidth * kVideoHeight);
}

void PafDecoder::DecodeVideoFrameOp2(const uint8_t *src) {
	int page = *src++;
	if (page != m_currentVideoPage) {
		memcpy(m_videoPages[m_currentVideoPage], m_videoPages[page], kVideoWidth * kVideoHeight);
	}
}

void PafDecoder::DecodeVideoFrameOp4(const uint8_t *src) {
	uint8_t *dst = m_videoPages[m_currentVideoPage];
	src += 2;
	int size = kVideoWidth * kVideoHeight;
	while (size != 0) {
		int8_t code = *src++;
		const int count = ABS(code) + 1;
		if (code < 0) {
			uint8_t color = *src++;
			memset(dst, color, count);
		} else {
			memcpy(dst, src, count);
			src += count;
		}
		dst += count;
		size -= count;
	}
}
