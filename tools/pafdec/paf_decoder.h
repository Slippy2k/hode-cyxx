
//
// Heart Of Darkness PAF Decoder
// Copyright (c) 2007 Gregory Montoir
//

#ifndef PAF_DECODER_H__
#define PAF_DECODER_H__

#include "intern.h"
#include "file.h"

struct ImageWriter;
struct SoundWriter;

struct PafHeader {
	uint32_t preloadFrameBlocksCount;
	uint32_t *frameBlocksCountTable;
	uint32_t *framesOffsetTable;
	uint32_t *frameBlocksOffsetTable;
	uint32_t framesCount;
	uint32_t frameBlocksCount;
	uint32_t startOffset;
	uint32_t readBufferSize;
	uint32_t maxVideoFrameBlocksCount;
	uint32_t maxAudioFrameBlocksCount;
};

class PafDecoder {
public:

	static const int kMaxVideosCount = 50;
	static const int kVideoWidth = 256;
	static const int kVideoHeight = 192;
	static const int kSoundBufferSize = 4922; // 256 * 2 + 2205 * 2

	bool Open(const char *filename, int videoNum);
	void Close();
	void Decode();

private:

	void SeekToVideo(int videoNum);
	void AlignReadHeaderTable(uint32_t *dst, int count);
	bool ReadPafHeader();
	void DecodeAudioFrame(const uint8_t *src, uint32_t dstOffset);
	void DecodeAudioFrame2205(const uint8_t *src);
	void DecodeVideoFrame(const uint8_t *src);
	uint8_t *GetVideoPageOffset(uint8_t a, uint8_t b);
	void DecodeVideoFrameOp0(const uint8_t *base, const uint8_t *src, uint8_t code);
	void DecodeVideoFrameOp1(const uint8_t *src);
	void DecodeVideoFrameOp2(const uint8_t *src);
	void DecodeVideoFrameOp4(const uint8_t *src);

	bool m_opened;
	File m_file;
	int m_videoNum;
	uint32_t m_videoOffset;
	PafHeader m_pafHdr;
	int m_currentVideoPage;
	uint8_t *m_videoPages[4];
	uint8_t m_paletteBuffer[256 * 3];
	uint8_t m_bufferBlock[2048];
	uint8_t *m_demuxVideoFrameBlocks;
	uint8_t *m_demuxAudioFrameBlocks;
	ImageWriter *m_imageWriter;
	SoundWriter *m_soundWriter;
};

#endif // PAF_DECODER_H__
