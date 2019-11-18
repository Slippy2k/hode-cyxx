
PANDOC_OPTS := \
	-f markdown \
	-V linkcolor:blue \
	-V geometry:a4paper \
	-V geometry:margin=2cm \
	-V mainfont="DejaVu Serif" \
	-V monofont="DejaVu Sans Mono" \
	--latex-engine=xelatex

all: HOD_PSX.pdf HOD_Win32.pdf

HOD_PSX.pdf: psx.md
	pandoc $^ $(PANDOC_OPTS) -o $@
	exiftool -all= $@
	qpdf --linearize $@ $@.tmp
	mv $@.tmp $@

HOD_Win32.pdf: win32.md
	pandoc $^ $(PANDOC_OPTS) -o $@
	exiftool -all= $@
	qpdf --linearize $@ $@.tmp
	mv $@.tmp $@
