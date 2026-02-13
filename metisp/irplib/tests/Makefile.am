## Process this file with automake to produce Makefile.in

##   This file is part of the IRPLIB package
##   Copyright (C) 2002,2003 European Southern Observatory

##   This library is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 2 of the License, or
##   (at your option) any later version.

##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.

##   You should have received a copy of the GNU General Public License
##   along with this program; if not, write to the Free Software
##   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02111-1307  USA

AUTOMAKE_OPTIONS = 1.8 foreign

DISTCLEANFILES = *~


if MAINTAINER_MODE

MAINTAINERCLEANFILES = Makefile.in

endif

AM_CPPFLAGS = $(all_includes)

LDADD = $(LIBIRPLIB) $(LIBCFITSIO) $(LIBCPLCORE) $(LIBCPLUI) $(LIBCPLDFS) $(LIBCPLDRS)

pkginclude_HEADERS =

noinst_HEADERS =

check_PROGRAMS = irplib_wlxcorr-test \
                 irplib_hist-test \
                 irplib_polynomial-test \
                 irplib_plugin-test \
                 irplib_utils-test \
                 irplib_framelist-test \
                 irplib_wcs-test \
                 irplib_strehl-test \
                 irplib_cat-test \
                 irplib_sdp_spectrum-test

irplib_wlxcorr_test_SOURCES = irplib_wlxcorr-test.c
irplib_wlxcorr_test_LDFLAGS = $(CPL_LDFLAGS)
irplib_wlxcorr_test_LDADD = $(LDADD)

irplib_hist_test_SOURCES = irplib_hist-test.c
irplib_hist_test_LDFLAGS = $(CPL_LDFLAGS)
irplib_hist_test_LDADD = $(LDADD)

irplib_polynomial_test_SOURCES = irplib_polynomial-test.c
irplib_polynomial_test_LDFLAGS = $(CPL_LDFLAGS)
irplib_polynomial_test_LDADD = $(LDADD)

irplib_plugin_test_SOURCES = irplib_plugin-test.c
irplib_plugin_test_LDFLAGS = $(CPL_LDFLAGS)
irplib_plugin_test_LDADD = $(LDADD)

irplib_utils_test_SOURCES = irplib_utils-test.c
irplib_utils_test_LDFLAGS = $(CPL_LDFLAGS)
irplib_utils_test_LDADD = $(LDADD)

irplib_framelist_test_SOURCES = irplib_framelist-test.c
irplib_framelist_test_LDFLAGS = $(CPL_LDFLAGS) 
irplib_framelist_test_LDADD = $(LDADD)

irplib_wcs_test_SOURCES = irplib_wcs-test.c
irplib_wcs_test_LDFLAGS = $(CPL_LDFLAGS) $(WCSLIB_LDFLAGS)
irplib_wcs_test_LDADD = $(LDADD) $(LIBWCSLIB)

irplib_cat_test_SOURCES = irplib_cat-test.c
irplib_cat_test_LDFLAGS = $(CPL_LDFLAGS) $(WCSLIB_LDFLAGS)
irplib_cat_test_LDADD = $(LDADD) $(LIBWCSLIB)

irplib_strehl_test_SOURCES = irplib_strehl-test.c
irplib_strehl_test_LDFLAGS = $(CPL_LDFLAGS) $(WCSLIB_LDFLAGS)
irplib_strehl_test_LDADD = $(LDADD) $(LIBWCSLIB)

irplib_sdp_spectrum_test_SOURCES = irplib_sdp_spectrum-test.c
irplib_sdp_spectrum_test_LDFLAGS = $(CPL_LDFLAGS)
irplib_sdp_spectrum_test_LDADD = $(LDADD)

TESTS          = $(check_PROGRAMS)

# Be sure to reexport important environment variables.
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
        CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
        LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
        OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
        MALLOC_PERTURB_=137 MALLOC_CHECK_=2

# We need to remove any files that the above tests created.
clean-local: 
	 $(RM) *.log *.paf *.fits .logfile

