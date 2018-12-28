set -x
mkdir out/
for f in saturn_loading_*png; do
	../../libxbr-standalone/test_app $f out/$f xbr2x
done
ffmpeg -framerate 10 -pattern_type glob -i 'out/*.png' -c:v libx264 hod_saturn_loading.mp4
