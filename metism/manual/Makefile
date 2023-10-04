#
# Builds the METIS pipeline manual from latex sources
#

instrument = metis

version = $(shell grep '\\newcommand{\\release}' $(TEX_SOURCES) | \
            head -n 1 | sed -e 's/.*{\(.*\)}/\1/' )

DOCDIR = ..

USER_MANUAL = $(instrument)-pipeline-manual-$(version).pdf


CHMOD = chmod
LATEX = pdflatex
MV = mv -f

CLEANFILES = *.aux *.idx *.log *.toc *.out *.bbl *.blg *.brf


TEX_MASTER = metis-pipeline-manual

TEX_SOURCES = \
	$(TEX_MASTER).tex \
	layout.tex \
	shortcut.tex \
	metis_abbreviations.tex \
	metis_algorithms.tex \
	metis_ancdata.tex \
	metis_installation.tex \
	metis_instrument.tex \
	metis_introduction.tex \
	metis_overview.tex \
	metis_knownissues.tex \
	metis_quickstart.tex \
	metis_rawdata.tex \
	metis_recipes.tex \
	metis_reduction.tex \
	metis_troubleshooting.tex

TEX_EXTRA_SOURCES = \
	texmf/eso-pdm.sty

TEX_FIGURES = $(wildcard figures/*.pdf figures/*.png figures/*.jpg)
TEX_OUTPUT = $(addsuffix .pdf, $(TEX_MASTER))
TEX_LOG = $(addsuffix .log, $(TEX_MASTER))
TEX_MAX_ITERATIONS = 5
TEX_OPTIONS := -file-line-error -halt-on-error -interaction nonstopmode

TEXINPUTS := .:./texmf//:./pipedoc//:

all: $(USER_MANUAL)

$(USER_MANUAL): $(TEX_OUTPUT)
	@echo $(MV) $(TEX_OUTPUT) $(DOCDIR)/$(USER_MANUAL); \
	$(MV) $(TEX_OUTPUT) $(DOCDIR)/$(USER_MANUAL)

$(TEX_OUTPUT): $(TEX_SOURCES) $(TEX_EXTRA_SOURCES) $(TEX_FIGURES)
	@echo "$(LATEX) $<"; \
	(rerun=1; i=0; \
	 until test $$i -eq $(TEX_MAX_ITERATIONS) -o $$rerun -ne 1; do \
	     if test -n "$(VERBOSE)"; then \
	         (TEXINPUTS=$(TEXINPUTS) $(LATEX) $(TEX_OPTIONS) $<) || \
			 exit 1; \
	     else \
		     (TEXINPUTS=$(TEXINPUTS) $(LATEX) $(TEX_OPTIONS) $< >/dev/null) || \
			 exit 1; \
		 fi; \
		 if test $$i -eq 0; then \
	     	if test -n "$(VERBOSE)"; then \
		 		bibtex $(TEX_MASTER); \
			else \
		 		bibtex $(TEX_MASTER) >/dev/null; \
			fi; \
			if test -s $(TEX_MASTER).bbl; then \
				bibtex $(TEX_MASTER) >/dev/null; \
			fi; \
		 fi; \
	     i=$$((i+1)); \
		 (egrep 'Label\(s\) may have changed' $(TEX_LOG) >/dev/null || \
		 egrep 'rerunfilecheck Warning' $(TEX_LOG) >/dev/null) || rerun=0; \
	 done) || exit 1; \
	$(CHMOD) 644 $(TEX_OUTPUT)

clean:
	@echo $(RM) $(USER_MANUAL); \
	$(RM) $(DOCDIR)/$(USER_MANUAL) $(TEX_OUTPUT); \
	if test -n "$(VERBOSE)"; then \
	    echo $(RM) $(CLEANFILES); \
	fi; \
	$(RM) $(CLEANFILES)


.PHONY: clean

