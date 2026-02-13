## Process this file with automake to produce Makefile.in

##   This file is part of the ESO Common Pipeline Library
##   Copyright (C) 2012,2013 European Southern Observatory

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

DISTCLEANFILES = *~

if MAINTAINER_MODE

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

endif

AM_CPPFLAGS = $(CPL_INCLUDES) $(HDRL_INCLUDES) -DHDRL_USE_PRIVATE $(GSL_INCLUDES)
AM_LDFLAGS = $(CPL_LDFLAGS) $(HDRL_LDFLAGS) $(GSL_LDFLAGS)

LDADD = ../libhdrl.la $(LIBCPLCORE) $(LIBCPLDFS) $(LIBCPLDRS) $(LIBCPLUI) $(LIBCEXT) $(GSL_LIBS) -lm
 
check_PROGRAMS = hdrl_buffer-test \
		 hdrl_overscan-test \
		 hdrl_sigclip-test \
		 hdrl_utils-test \
		 hdrl_combine-test \
		 hdrl_bpm_2d-test \
		 hdrl_bpm_3d-test \
		 hdrl_bpm_fit-test \
		 hdrl_bpm_utils-test \
		 hdrl_collapse-test \
		 hdrl_image-test \
		 hdrl_imagelist_io-test \
		 hdrl_imagelist_basic-test \
		 hdrl_imagelist_view-test \
		 hdrl_elemop-test \
		 hdrl_parameter-test \
		 hdrl_fit-test \
		 hdrl_strehl-test \
		 hdrl_lacosmics-test \
		 hdrl_fringe-test \
		 hdrl_catalogue-test \
		 hdrl_random-test \
		 hdrl_frameiter-test \
		 hdrl_flat-test \
		 hdrl_spectrum1d-test \
		 hdrl_spectrum1d_shift-test \
		 hdrl_spectrum1dlist-test \
		 hdrl_efficiency-test \
		 hdrl_response-test \
		 hdrl_der_snr-test \
		 hdrl_dar-test \
		 hdrl_fpn-test \
		 hdrl_prototyping-test \
		 hdrl_multiiter-test \
		 hdrl_correlation-test \
		 hdrl_mode-test \
		 hdrl_resample-test \
		 hdrl_download-test \
		 hdrl_barycorr-test \
		 hdrl_maglim-test

hdrl_overscan_test_SOURCES = hdrl_overscan-test.c
hdrl_sigclip_test_SOURCES = hdrl_sigclip-test.c
hdrl_utils_test_SOURCES = hdrl_utils-test.c
hdrl_combine_test_SOURCES = hdrl_combine-test.c
hdrl_bpm_2d_test_SOURCES = hdrl_bpm_2d-test.c
hdrl_bpm_3d_test_SOURCES = hdrl_bpm_3d-test.c
hdrl_bpm_fit_test_SOURCES = hdrl_bpm_fit-test.c
hdrl_bpm_utils_test_SOURCES = hdrl_bpm_utils-test.c
hdrl_collapse_test_SOURCES = hdrl_collapse-test.c
hdrl_image_test_SOURCES = hdrl_image-test.c
hdrl_imagelist_io_test_SOURCES = hdrl_imagelist_io-test.c
hdrl_imagelist_basic_test_SOURCES = hdrl_imagelist_basic-test.c
hdrl_imagelist_view_test_SOURCES = hdrl_imagelist_view-test.c
hdrl_elemop_test_SOURCES = hdrl_elemop-test.c
hdrl_parameter_test_SOURCES = hdrl_parameter-test.c
hdrl_buffer_test_SOURCES = hdrl_buffer-test.c
hdrl_fit_test_SOURCES = hdrl_fit-test.c
hdrl_strehl_test_SOURCES = hdrl_strehl-test.c
hdrl_lacosmics_test_SOURCES = hdrl_lacosmics-test.c
hdrl_fringe_test_SOURCES = hdrl_fringe-test.c
hdrl_catalogue_test_SOURCES = hdrl_catalogue-test.c
hdrl_random_test_SOURCES = hdrl_random-test.c
hdrl_frameiter_test_SOURCES = hdrl_frameiter-test.c
hdrl_flat_test_SOURCES = hdrl_flat-test.c
hdrl_spectrum1d_test_SOURCES = hdrl_spectrum1d-test.c
hdrl_spectrum1dlist_test_SOURCES = hdrl_spectrum1dlist-test.c
hdrl_der_snr_test_SOURCES = hdrl_der_snr-test.c
hdrl_efficiency_test_SOURCES = hdrl_efficiency-test.c
hdrl_response_test_SOURCES = hdrl_response-test.c
hdrl_correlation_test_SOURCES = hdrl_correlation-test.c
hdrl_spectrum1d_shift_test_SOURCES = hdrl_spectrum1d_shift-test.c
hdrl_dar_test_SOURCES = hdrl_dar-test.c
hdrl_fpn_test_SOURCES = hdrl_fpn-test.c
hdrl_prototyping_test_SOURCES = hdrl_prototyping-test.c
hdrl_multiiter_test_SOURCES = hdrl_multiiter-test.c
hdrl_resample_test_SOURCES = hdrl_resample-test.c
hdrl_mode_test_SOURCES = hdrl_mode-test.c
hdrl_maglim_test_SOURCES = hdrl_maglim-test.c
hdrl_download_test_SOURCES = hdrl_download-test.c
hdrl_barycorr_test_SOURCES = hdrl_barycorr-test.c

# Be sure to reexport important environment variables.
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
	CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
	LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
	OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
	MALLOC_PERTURB_=31 MALLOC_CHECK_=2 \
	GOMP_SPINCOUNT=10000

TESTS = $(check_PROGRAMS) 

# Expected failures
XFAIL_TESTS =

# We need to remove any files that the above tests created.
clean-local:
	$(RM) *.fits *.tfits *.log finals2000A.data
