set -x

dir=$HOME/Data/heart_of_darkness/DATA_fr/
levels="rock fort pwr1 isld lava pwr2 lar1 lar2 dark";

for name in $levels; do
	mst=${dir}/${name}_hod.mst
	out=bytecode/${name}_hod.mst.txt
	./hod_decoder/hod_decoder $mst > $out
	sss=${dir}/${name}_hod.SSS.bytecode
	out=bytecode/${name}_hod.sss.txt
	./decode_sss $sss > $out
done
