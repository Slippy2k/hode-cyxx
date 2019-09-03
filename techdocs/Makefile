
#HODwin32.pdf: HODwin32.md.html
#	google-chrome --headless --print-to-pdf=$@ $^

PANDOC_OPTS := \
	-f markdown \
	--toc \
	-V linkcolor:blue \
	-V geometry:a4paper \
	-V geometry:margin=2cm \
	-V mainfont="DejaVu Serif" \
	-V monofont="DejaVu Sans Mono" \
	--latex-engine=xelatex

all: HOD_PSX.pdf HOD_Win32.pdf

HOD_PSX.pdf: PSX.md
	pandoc $^ $(PANDOC_OPTS) -o $@

HOD_Win32.pdf: HODwin32.md
	pandoc $^ $(PANDOC_OPTS) -o $@
