## Process this file with automake to produce Makefile.in

##   This file is part of the ESO Common Pipeline Library
##   Copyright (C) 2001-2008 European Southern Observatory

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
##   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


AUTOMAKE_OPTIONS = 1.8 foreign

DISTCLEANFILES = *~


if MAINTAINER_MODE

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

endif

AM_CPPFLAGS = $(all_includes) -DCX_LOG_DOMAIN=\"CasuLib\" -I$(top_srcdir) -DHDRL_USE_PRIVATE
AM_LDFLAGS = $(CPL_LDFLAGS)
LDADD = ../libcatalogue.la  $(top_builddir)/libhdrl.la $(LIBCPLCORE) $(LIBCPLDRS) $(LIBCEXT) -lm

check_PROGRAMS = hdrl_cat_conf-test \
                 hdrl_cat_background-test \
	             hdrl_cat_utils_sort-test \
	             hdrl_cat_apio-test \
	             hdrl_cat_classify-test \
	             hdrl_cat_phopt-test \
	             hdrl_cat_overlp-test \
	             hdrl_cat_radii-test \
	             hdrl_cat_filter-test \
	             hdrl_cat_terminate-test \
	             hdrl_cat_moments-test \
	             hdrl_cat_areals-test \
	             hdrl_cat_seeing-test \
	             hdrl_cat_polynm-test \
	             hdrl_cat_casu-test \
	             hdrl_cat_casu_bkg_badpatch-test \
	             hdrl_cat_casu_addmul-test \
	             hdrl_cat_sim_montecarlo-test

hdrl_cat_conf_test_SOURCES = hdrl_cat_conf-test.c
hdrl_cat_background_test_SOURCES = hdrl_cat_background-test.c
hdrl_cat_utils_sort_test_SOURCES = hdrl_cat_utils_sort-test.c
hdrl_cat_apio_test_SOURCES = hdrl_cat_apio-test.c
hdrl_cat_classify_test_SOURCES = hdrl_cat_classify-test.c
hdrl_cat_phopt_test_SOURCES = hdrl_cat_phopt-test.c
hdrl_cat_overlp_test_SOURCES = hdrl_cat_overlp-test.c
hdrl_cat_radii_test_SOURCES = hdrl_cat_radii-test.c
hdrl_cat_filter_test_SOURCES = hdrl_cat_filter-test.c
hdrl_cat_terminate_test_SOURCES = hdrl_cat_terminate-test.c
hdrl_cat_moments_test_SOURCES = hdrl_cat_moments-test.c
hdrl_cat_areals_test_SOURCES = hdrl_cat_areals-test.c
hdrl_cat_seeing_test_SOURCES = hdrl_cat_seeing-test.c
hdrl_cat_polynm_test_SOURCES = hdrl_cat_polynm-test.c
hdrl_cat_casu_test_SOURCES = hdrl_cat_casu-test.c
hdrl_cat_casu_bkg_badpatch_test_SOURCES = hdrl_cat_casu_bkg_badpatch-test.c
hdrl_cat_casu_addmul_test_SOURCES = hdrl_cat_casu_addmul-test.c
hdrl_cat_sim_montecarlo_test_SOURCES = hdrl_cat_sim_montecarlo-test.c

TESTS = hdrl_cat_conf-test \
        hdrl_cat_background-test \
        hdrl_cat_utils_sort-test \
        hdrl_cat_apio-test \
        hdrl_cat_classify-test \
        hdrl_cat_phopt-test \
        hdrl_cat_overlp-test \
        hdrl_cat_radii-test \
        hdrl_cat_filter-test \
        hdrl_cat_terminate-test \
        hdrl_cat_moments-test \
        hdrl_cat_areals-test \
        hdrl_cat_seeing-test \
        hdrl_cat_polynm-test \
        hdrl_cat_casu-test \
        hdrl_cat_casu_bkg_badpatch-test \
        hdrl_cat_casu_addmul-test \
        hdrl_cat_sim_montecarlo-test

# Be sure to reexport important environment variables.
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
	CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
	LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
	OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
	MALLOC_PERTURB_=31 MALLOC_CHECK_=2 \
	GOMP_SPINCOUNT=10000

XFAIL_TESTS =

# We need to remove any files that the above tests created.
clean-local:
	$(RM) *.fits *.log
