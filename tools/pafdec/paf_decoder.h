
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
	uint32 preloadFrameBlocksCount;
	uint32 *frameBlocksCountTable;
	uint32 *framesOffsetTable;
	uint32 *frameBlocksOffsetTable;
	uint32 framesCount;
	uint32 frameBlocksCount;
	uint32 startOffset;
	uint32 readBufferSize;
	uint32 maxVideoFrameBlocksCount;
	uint32 maxAudioFrameBlocksCount;
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
	void AlignReadHeaderTable(uint32 *dst, int count);
	bool ReadPafHeader();
	void DecodeAudioFrame(const uint8 *src, uint32 dstOffset);
	void DecodeAudioFrame2205(const uint8 *src);
	void DecodeVideoFrame(const uint8 *src);
	uint8 *GetVideoPageOffset(uint8 a, uint8 b);
	void DecodeVideoFrameOp0(const uint8 *base, const uint8 *src, uint8 code);
	void DecodeVideoFrameOp1(const uint8 *src);
	void DecodeVideoFrameOp2(const uint8 *src);
	void DecodeVideoFrameOp4(const uint8 *src);

	bool m_opened;
	File m_file;
	int m_videoNum;
	uint32 m_videoOffset;
	PafHeader m_pafHdr;
	int m_currentVideoPage;
	uint8 *m_videoPages[4];
	uint8 m_paletteBuffer[256 * 3];
	uint8 m_bufferBlock[2048];
	uint8 *m_demuxVideoFrameBlocks;
	uint8 *m_demuxAudioFrameBlocks;
	ImageWriter *m_imageWriter;
	SoundWriter *m_soundWriter;
};

#endif // PAF_DECODER_H__
