VERSION=1.5.0

all: 
	$(MAKE) hdrlpdoc.pdf 
	$(MAKE) pdf_check 
	$(MAKE) mv

hdrlpdoc.pdf: *.tex hdrlpdoc.bib

clean:
	rm -f *.dvi *.aux *.toc *.out *.log *.idx *.pdf *.ps *.bbl *.blg *.brf

# Check that all clickable URIs in PDF-document are valid
pdf_check: hdrlpdoc.pdf 
	perl -nle '$$done{$$1}++ or system("wget --no-check-certificate -q -O /dev/null $$1") ? die $$1 : print $$1 while s/\bURI\/URI\(([fht]+tp[^\)]+)\)//i' hdrlpdoc.pdf

mv: hdrlpdoc.pdf 
	mv hdrlpdoc.pdf hdrl-manual-$(VERSION).pdf

view: 
	evince hdrl-manual-$(VERSION).pdf

%.pdf: %.tex
	pdflatex -draftmode $*
	bibtex   $*
	pdflatex $*
	-grep 'Label.s. may have changed' $*.log && echo s|pdflatex $*
	-grep 'Label.s. may have changed' $*.log && echo s|pdflatex $*
	-perl -e 'system("grep -w Warning $*.log") || die "Warnings are not allowed\n"'
