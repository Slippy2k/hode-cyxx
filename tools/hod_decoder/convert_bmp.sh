#set -x
for f in *bmp; do
	name=$( basename $f .bmp )
	optipng -o9 -strip all -out $name.png $f
	rm $f
done
