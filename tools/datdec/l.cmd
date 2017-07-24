@echo off

make

lvl_decoder.exe ../../DATA/ROCK_HOD.LVL

rem convert -depth 8 -size 256x192 gray:bmp_20_0.bin bmp_20_0.png
rem convert -depth 8 -size 256x192 gray:bmp_21_0.bin bmp_21_0.png
rem convert -depth 8 -size 256x192 gray:bmp_22_0.bin bmp_22_0.png
rem convert -depth 8 -size 256x192 gray:bmp_23_0.bin bmp_23_0.png
rem convert -depth 8 -size 256x192 gray:bmp_24_0.bin bmp_24_0.png
rem convert -depth 8 -size 256x192 gray:bmp_25_0.bin bmp_25_0.png
rem convert -depth 8 -size 256x192 gray:bmp_26_0.bin bmp_26_0.png

rem lvl_decoder.exe "L:\games\Heart Of Darkness\Dark_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Fort_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Isld_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Lar1_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Lar2_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Lava_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Pwr1_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Pwr2_hod.lvl"
rem lvl_decoder.exe "L:\games\Heart Of Darkness\Rock_hod.lvl"
