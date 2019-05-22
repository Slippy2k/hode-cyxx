python transform.py
fn=pwr1_transform.gif
convert -delay 10 -loop 0 -layers Optimize transform_*png $fn.gif
