
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

mdec_coeffs.h: generate_mdec_coeffs.py
	python generate_mdec_coeffs.py > $@

clean:
	rm -f *.o hod_decoder
