set -x

ZLIB=$HOME/Install/zlib-1.2.11
LIBJPEG=$HOME/Install/jpeg-9c
SOURCES="decode_sss.cpp decode_mst.cpp fileio.cpp lzw.cpp main.cpp mdec.cpp screenshot.cpp wav.cpp"

OUT=hod_decoder.exe
i686-w64-mingw32-g++ -Wall -Os -I $ZLIB/ -I $LIBJPEG/ -o $OUT $SOURCES -L $ZLIB/ -lz -L $LIBJPEG/.libs/ -ljpeg -static-libgcc -static-libstdc++
i686-w64-mingw32-strip $OUT
upx -9 $OUT
