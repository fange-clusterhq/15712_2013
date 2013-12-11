
PAPER = report

TEX = $(wildcard ./*.tex)

.PHONY: all, clean
all: $(TEX)
	pdflatex $(PAPER)

clean:
	rm -rf *.dvi *.log *.aux *.bbl *.blg *.pdf *.toc
