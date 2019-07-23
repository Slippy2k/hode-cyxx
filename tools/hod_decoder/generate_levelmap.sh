set -x

DATADIR=~/Data/heart_of_darkness/DATA_retail/

do_levelmap () {
	./hod_decoder $DATADIR/$1.lvl > levelmap_$1.txt
	python generate_levelmap.py levelmap_$1.txt
	optipng -o9 -strip all levelmap_$1.png
}

do_levelmap 'rock_hod'
do_levelmap 'fort_hod'
do_levelmap 'pwr1_hod'
do_levelmap 'isld_hod'
do_levelmap 'lava_hod'
do_levelmap 'pwr2_hod'
do_levelmap 'lar1_hod'
do_levelmap 'lar2_hod'
do_levelmap 'dark_hod'
