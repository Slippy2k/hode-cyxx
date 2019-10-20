
CPPFLAGS += -Wall -g

LIBBS_DIR = libbs
LIBBS_OBJS = $(addprefix $(LIBBS_DIR)/, bs.o idctfst.o jfdctint.o vlc.o)
CPPFLAGS += -I$(LIBBS_DIR) -DWITH_LIBBS

hod_decoder: decode_sss.o decode_mst.o fileio.o lzw.o main.o mdec_ffmpeg.o screenshot.o wav.o $(LIBBS_OBJS)
	$(CXX) -o $@ $^ -lz -lavcodec -lavutil -ljpeg

clean:
	rm -f *.o $(LIBBS_OBJS) hod_decoder
