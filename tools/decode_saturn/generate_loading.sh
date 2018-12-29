set -x
mkdir out/
for f in saturn_loading_*png; do
	../../libxbr-standalone/test_app $f out/$f xbr2x
done
fn=HoD_Saturn_Loading
ffmpeg -framerate 10 -pattern_type glob -i 'out/*.png' -c:v libx264 $fn.mp4
convert -delay 10 -loop 0 -layers Optimize 'out/*png $fn.gif
