
#include "mdec.h"
extern "C" {
	#include <libavcodec/avcodec.h>
}

int decodeMDEC(const uint8_t *src, int len, int w, int h, const void *userdata, void (*output)(const MdecOutput *, const void *)) {
	static bool codec_inited = false;

	if (!codec_inited) {
		avcodec_register_all();
		codec_inited = true;
	}

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
		MdecOutput mdecOutput;
		mdecOutput.format = kMdecOutputYuv;
		mdecOutput.w = frame->width;
		mdecOutput.h = frame->height;
		for (int i = 0; i < 3; ++i) {
			mdecOutput.planes[i].ptr = frame->data[i];
			mdecOutput.planes[i].pitch = frame->linesize[i];
		}
		output(&mdecOutput, userdata);
	}
	avcodec_free_context(&ctx);
	av_frame_free(&frame);

	return ret;
}
