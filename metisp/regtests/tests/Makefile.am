## Process this file with automake to produce Makefile.in

##   This file is part of the ESOTK Toolkit
##   Copyright (C) 2016 European Southern Observatory

##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 2 of the License, or
##   (at your option) any later version.

##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.

##   You should have received a copy of the GNU General Public License
##   along with this program; if not, write to the Free Software
##   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = 1.8 foreign

TESTS ?= $(wildcard ../../${PACKAGE}_reg/*.yaml *.yaml)

GEN ?= "eso-testgen.py"
GOLD ?= regtest_results
NEW ?= regtest_workspace
RUNSCRIPTS ?= $(patsubst %.yaml,%.mk, $(TESTS))
CUR_DIR ?= $(shell pwd)

DISTCLEANFILES = *~ *.mk 

export ESOREX_PLUGIN_DIR=$(CURDIR)/../../recipes/.libs

all:
	echo "Run <make build_mk> to generate the .mk files or <make regcheck> to execute the full regressions"

check:
	echo "Run <make build_mk> to generate the .mk files or <make regcheck> to execute the full regressions"

validate: $(TESTS)
	if python -c 'import jsonschema' 2>/dev/null; then $(GEN) ${REGCHECK_FLAGS} --validate $^; fi

build_mk: validate $(RUNSCRIPTS)

#generate NRI/p_regtest compatible recipe configurations
create_sof: $(TESTS) validate
	$(GEN) ${REGCHECK_FLAGS} --flatten  $(TESTS)

SUFFIXES = .yaml .mk
.yaml.mk:
	d="`basename $^`"; \
	outscript="`basename $@`"; \
	$(GEN) ${REGCHECK_FLAGS} --forgive -o "$(NEW)/$$d" -g "$(GOLD)/$$d" -r "$$outscript" -t makefile "$^"

# run test scripts and generate or verify against a gold dataset
regcheck: validate $(RUNSCRIPTS)
	@if test ! -d "${SOF_DATA}" ; then \
                echo "ERROR: The SOF_DATA environment variable was not set to point to the regression data." 1>&2 ; \
                exit 1 ; \
        fi
	## Done to always move the final products into the GOLD directory 
	@if test -d "$(GOLD)" ; then \
                echo "Cleaning up "$(GOLD)" directory for new execution" ; \
                $(RM) -r $(GOLD) ; \
        fi
	set -e && for r in $(RUNSCRIPTS);do \
	    $(MAKE) -f `basename $$r`; \
	done

valgrind: validate
	set -e && for r in $(TESTS); do \
	    $(GEN) ${REGCHECK_FLAGS} --valgrind -o valgrind/`basename $$r` -r `basename $$r`-valgrind.mk -t makefile $$r; \
	    $(MAKE) -f `basename $$r`-valgrind.mk; \
	done

# compare already existing test results with fitsdiff-folder
compare:
	@if test ! -d "${DATA_REF}" ; then \
                echo "ERROR: The DATA_REF environment variable must be set to point to the reference regression results data." 1>&2 ; \
                exit 1 ; \
        fi
	set -e && for r in $(TESTS); do \
	  [ ! -e $(GOLD)/`basename $$r` ] && echo "$(GOLD)/`basename $$r` missing run check first" && continue; \
	  fitsdiff-folder.sh --late-fail ${DATA_REF}/`basename $$r` $(GOLD)/`basename $$r` \
	    --ignore-keywords='DATE,ESO PRO REC*,DATAMD5,CHECKSUM,DATASUM'; \
	done

# create pipeline regression report in xml and html formats
# the following environment variables must be set
# DATA_REF : with reference results of pipeline products created by make check
# PIPETOOLS_DIR : directory containing pipetools project


report:
	## test required env variables are set, else exit with error message
	@if test ! -d "${DATA_REF}" ; then \
                echo "ERROR: The DATA_REF environment variable must be set to point to the reference regression results data." 1>&2 ; \
                exit 1 ; \
        fi

	@if test ! -d "${PIPETOOLS_DIR}" ; then \
                echo "ERROR: The PIPETOOLS_DIR environment variable must be set to point to the directory where the SVN pipetools project is checkout." 1>&2 ; \
                exit 1 ; \
        fi

	## set additional required env variables
	export DATA_TEST=$(CUR_DIR)/$(subst ",,$(GOLD)) ; \
	ln -s ../../${PACKAGE}_reg/*.yaml . ; \
	export DATA_YAML_DIR=$(CUR_DIR) ; \
	cd ${PIPETOOLS_DIR}/pipeline_regtest_py/ ; \
	python -m pytest pipeline_reg_tests*.py ${REGTESTS_TO_EXECUTE} -v --tb=short --junit-xml=regression_test_results.xml --html=regression_html_results.html --self-contained-html ; \
	cp regression_test_results.xml regression_html_results.html $(CUR_DIR) ; \
	cd -



# We need to remove any files that the above tests created.
clean-local:
	$(RM) *.fits *.tfits *.log *.yaml *.mk 
	$(RM) -r valgrind $(GOLD) $(NEW) sof


if MAINTAINER_MODE
MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
endif


.PHONY: all check clean distclean validate link_yaml
.NOTPARALLEL:
