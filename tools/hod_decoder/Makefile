
CPPFLAGS += -Wall -g

MDEC := default

ifeq "$(MDEC)" "libbs"
	LIBBS_DIR := libbs
	CPPFLAGS  += -I$(LIBBS_DIR)
	MDEC_OBJS := mdec_libbs.o $(addprefix $(LIBBS_DIR)/, bs.o idctfst.o jfdctint.o vlc.o)
	MDEC_LIBS :=
endif

ifeq "$(MDEC)" "ffmpeg"
	MDEC_OBJS := mdec_ffmpeg.o
	MDEC_LIBS := -lavcodec -lavutil
endif

ifeq "$(MDEC)" "oddlib"
	MDEC_OBJS := mdec_oddlib.o PSXMDECDecoder.o
	MDEC_LIBS :=
endif

ifeq "$(MDEC)" "default"
	MDEC_OBJS := mdec.o
	MDEC_LIBS :=
endif

hod_decoder: decode_sss.o decode_mst.o fileio.o lzw.o main.o screenshot.o wav.o $(MDEC_OBJS)
	$(CXX) -o $@ $^ -lz -ljpeg $(MDEC_LIBS)

mdec_ac_coeffs.h: generate_ac_huffman.py
	python generate_ac_huffman.py > $@

clean:
	rm -f *.o $(LIBBS_OBJS) hod_decoder
