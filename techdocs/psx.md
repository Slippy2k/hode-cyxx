
# Reverse engineering HoD PSX bitmaps

Heart of Darkness from Amazing Studio is known for its long development time.
When released in 1998, the game was available on both Windows and PlayStation platforms.

The Windows version uses a 256 colors palette for its graphics.
This is likely inherited from its early development years when DOS and VGA cards were the principal gaming configuration on PC.

The PlayStation version was released a few months after the Windows version. Let’s figure out the native graphics file format used.

## Taking a first guess

Loading the game in the emulator and playing the first few screens, some JPEG artifacts can be observed in some areas of the background bitmaps.

![PSX Emulator Screenshot 1](img/psx_emulator_1.png)

The PlayStation has a hardware block (MDEC) specialized in decoding JPEG like encoded frames. This is usually used by games for playing FMV sequences.

## Checking disassembly

Running strings and grep over the game executable show some left-over references to MDEC.

```
$ strings slus_006.96 | grep -i mdec
MDEC_rest:bad option(%d)
MDEC_in_sync
MDEC_out_sync
```

Going through the disassembly and comparing the assembly code with known libraries (such as libbs) allows to rename and map a few functions related to the MDEC unit.

```
.text:8005E184 DecDCTReset
.text:8005E420 DecDCTout
.text:800648D0 DecDCToutCallback
.text:80064900 DecDCTvlc
```

The DecDCTvlcfunction is especially interesting since this is where the MDEC compressed bitstream is fed to the hardware block.

```
.text:80064900 DecDCTvlc:
.text:80064918      bnez $a0, loc_80064954
```

## Breakpoint and memory dump

To confirm the MDEC is used for background bitmap, a breakpoint is set at 0x80064918 before moving Andy to the second screen.

![PSX Emulator Screenshot 2](img/psx_emulator_2.png)

The program pauses when Andy moves to the next screen. The contents of memory pointed at by the register A0 are

```
CD CD BC 00 00 00 20 33 00 38 01 00 02 00
```

Ignoring the first 6 bytes, the sequence appears to match the documented format a MDEC frame.

```
 0 . . . 2 . . little . Unknown
                          Number of run length codes in the frame?
                          Size of data following this header?
 2 . . . 2 . . little . Always 0x3800
 4 . . . 2 . . little . Frame quantization scale
 6 . . . 2 . . little . Version of the frame
 8 . . . . . . . . . .  Compressed macro blocks
                          Stream of 2 byte little-endian values
```

The contents of the memory are dumped to a file (100 KB) for conversion.

## Data dump decode

To confirm the data is a MDEC frame, we can feed it to a decoder.

ffmpeg has a MDEC decoder in libavcodec. ffmpeg actually supports playing back .STR files but this is not relevant here as only the codec code is necessary.

With a few lines of C, the data dump is correctly decoded.

```
const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MDEC);
AVCodecContext *ctx = avcodec_alloc_context3(codec);
ctx->width = 256;
ctx->height = 192;
avcodec_open2(ctx, codec, 0);

AVPacket pkt;
av_new_packet(&pkt, len);

AVFrame *frame = av_frame_alloc();
avcodec_decode_video2(ctx, frame, &hasFrame, &pkt);
```

After converting the decoding frame, the second screen of the first level can be displayed, without glitches.

This confirms the use of a MDEC compressed frame as the native format of the background bitmaps.
Next step is to extract all pictures from the game data files and convert them.

## Data files

The files found on the PlayStation CDs are similar to the ones from the PC Windows version.
For each level, there are two files :

* .LVL : contains the palettes, bitmaps, sprites and pre-calculated tables for shadows
* .MST : contains the bytecode and triggers for the monster logic

Scanning for MDEC frames in the .lvl files and feeding the data to the ffmpeg decoder results in some ac-texerrors, indicating damaged or wrong data.

```
Read 1048576 bytes at 0x1a330c
MDEC len 13088, VLC_ID 0x3800
qscale 1 version 2
avcodec_decode_video2 ret -1094995529
[mdec @ 0x55a04ab001a0] ac-tex damaged at 2 0
```

Forcing the output to a file and continuing on errors, some 8x8 blocks are correctly decoded while others are off (wrong position and/or colors).

![Decode Glitch](img/hod_psx_level1_screen2_glitch.png)

Comparing the data found in the files with the previous memory dump shows no difference at the beginning of the buffer.

```
$ cmp 001a330c.bss a0.bss
001a330c.bss a0.bss differ: byte 1265, line 8
```

There are four extra bytes in 001a330c.bss at 0x4F0 (1265-1) in the data files when comparing with the memory dump.

The original PC demo data files had a similar pattern : every 2048 bytes, the last 4 bytes corresponds to a checksum of the previous 2044 bytes.

```
uint32_t fioUpdateCRC(uint32_t sum, uint8_t *buf, uint32_t size) {
  assert((size & 3) == 0);
  size >>= 2;
  while (size--) {
    sum ^= READ_LE_UINT32(buf); buf += 4;
  }
  return sum;
}
void SectorFile::refillBuffer() {
  int size = fread(_buf, 1, 2048, _fp);
  if (size == 2048) {
    uint32_t crc = fioUpdateCRC(0, _buf, 2048);
    assert(crc == 0);
```

Calculating the address confirms these four extra bytes are aligned to 0x800.

```
$ printf %x $(( 0x001a330c + 0x4f0 ))
1a37fc
```

The extraction code is updated to skip four bytes every 2048 bytes read.
After feeding the data to the ffmpeg decoder, the second level screen can be decoded without any errors.

![Level1 Screen2 PSX](img/hod_psx_level1_screen2.png)

It is interesting to note the PSX data file use the same sector based file format for its data files.

## Batch conversion

With the file format understood, all of the level screens bitmaps can be extracted and compared with the PC version.

Below is the comparaison of the first three screens, on the left the Windows PC paletted 256 colors and on the right the PlayStation YUV bitmaps.

![Level1 PC PSX](img/psx_level1_screens_1_2_3.png)
