
CPPFLAGS += -Wall -g

MDEC := default

ifeq "$(MDEC)" "ffmpeg"
	MDEC_OBJS := mdec_ffmpeg.o
	MDEC_LIBS := -lavcodec -lavutil
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
	rm -f *.o hod_decoder
