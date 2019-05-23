
python transform.py pwr1_hod_01_0.bmp water
fn=pwr1_transform
convert -delay 10 -loop 0 -layers Optimize transform_*png $fn.gif
rm transform_*png

python transform.py lar1_hod_12_0.bmp fire
fn=lar1_transform
convert -delay 10 -loop 0 -layers Optimize transform_*png $fn.gif
rm transform_*png

python transform.py lava_hod_05_0.bmp fire
fn=lava_transform
convert -delay 10 -loop 0 -layers Optimize transform_*png $fn.gif
rm transform_*png
