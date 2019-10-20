#set -x

EXT=bmp
if [ $1 = "-psx" ]; then
	EXT=jpg
	shift
fi
DATA=$1

do_levelmap () {
	name=$1
	data=$2
	extension=$3
	./hod_decoder $data/$name.lvl > levelmap_$name.txt
	python generate_levelmap.py levelmap_$name.txt $extension
	optipng -o9 -strip all levelmap_$type_$name.png
}

do_files () {
	data=$1
	extension=$2
	do_levelmap 'rock_hod' $data $extension
	do_levelmap 'fort_hod' $data $extension
	do_levelmap 'pwr1_hod' $data $extension
	do_levelmap 'isld_hod' $data $extension
	do_levelmap 'lava_hod' $data $extension
	do_levelmap 'pwr2_hod' $data $extension
	do_levelmap 'lar1_hod' $data $extension
	do_levelmap 'lar2_hod' $data $extension
	do_levelmap 'dark_hod' $data $extension
}

do_files $DATA $EXT
