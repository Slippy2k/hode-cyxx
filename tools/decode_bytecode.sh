
dir=bytecode
levels="rock fort pwr1 isld lava pwr2 lar1 lar2 dark";

for name in $levels; do
	mst=${dir}/${name}_hod.MST.bytecode
	out=${dir}/${name}_hod.mst.txt
	./decode_mst $mst > $out
	sss=${dir}/${name}_hod.SSS.bytecode
	out=${dir}/${name}_hod.sss.txt
	./decode_sss $sss > $out
done
