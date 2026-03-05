#
# Builds the METIS Reflex tutorial from latex sources
#

instrument = metis

version = $(shell grep '\\dmdDocVersion' $(TEX_MASTER).tex | \
            head -n 1 | sed -e 's/.*{\(.*\)}/\1/' )

DOCDIR = ..

USER_MANUAL = $(instrument)-reflex-tutorial-$(version).pdf


CHMOD = chmod
LATEX = pdflatex
MV = mv -f

CLEANFILES = *.aux *.idx *.log *.toc *.out *.bbl *.blg *.brf


TEX_MASTER = metis_reflex_tutorial

TEX_SOURCES = \
	$(TEX_MASTER).tex \
	shortcut.tex \
	layout.tex

TEX_EXTRA_SOURCES = \
	texmf/dmd-doc.sty \
	pipedoc/reflex/reflex_about_canvas.tex \
	pipedoc/reflex/reflex_faq.tex \
	pipedoc/reflex/reflex_intro_scope.tex \
	pipedoc/reflex/reflex_lazy_mode.tex \
	pipedoc/reflex/reflex_simple_actors.tex \
	pipedoc/reflex/reflex_wkf_general_desc.tex \
	pipedoc/reflex/reflex_wkf_installation.tex \
	pipedoc/reflex/reflex_wkf_quickstart.tex

TEX_FIGURES = $(wildcard figures/*.pdf figures/*.png figures/*.jpg figures/*.jpeg pipedoc/figures/*.pdf pipedoc/figures/*.png)
TEX_OUTPUT = $(addsuffix .pdf, $(TEX_MASTER))
TEX_LOG = $(addsuffix .log, $(TEX_MASTER))
TEX_MAX_ITERATIONS = 5
TEX_OPTIONS := -file-line-error -halt-on-error -interaction nonstopmode

TEXINPUTS := .:./texmf//:./pipedoc//:.pipedoc/reflex//:

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

