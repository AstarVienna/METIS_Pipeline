## Process this file with automake to produce Makefile.in

##   This file is part of the irplib package
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

SUBDIRS = . tests


if MAINTAINER_MODE

MAINTAINERCLEANFILES = Makefile.in

endif

AM_CPPFLAGS = $(all_includes) $(GSL_INCLUDES) $(CFITSIO_INCLUDES) $(IRPLIB_CPPFLAGS)

noinst_HEADERS =    irplib_flat.h \
                    irplib_calib.h \
                    irplib_cat.h \
                    irplib_distortion.h \
                    irplib_fft.h \
                    irplib_framelist.h \
                    irplib_utils.h \
                    irplib_oddeven.h \
                    irplib_slitpos.h \
                    irplib_strehl.h \
                    irplib_stdstar.h \
                    irplib_plugin.h \
                    irplib_polynomial.h \
                    irplib_ppm.h \
                    irplib_spectrum.h \
                    irplib_match_cats.h \
                    irplib_hist.h \
                    irplib_wavecal.h \
                    irplib_wavecal_impl.h \
                    irplib_wcs.h \
                    irplib_wlxcorr.h \
                    irplib_mkmaster.h \
                    irplib_ksigma_clip.h \
                    irplib_ksigma_clip_body.h \
                    irplib_sdp_spectrum.h

pkginclude_HEADERS = 

noinst_LTLIBRARIES = libirplib.la

libirplib_la_SOURCES =  irplib_flat.c \
                        irplib_calib.c \
                        irplib_cat.c \
                        irplib_distortion.c \
                        irplib_fft.c \
                        irplib_framelist.c \
                        irplib_match_cats.c \
                        irplib_utils.c \
                        irplib_oddeven.c \
                        irplib_slitpos.c \
                        irplib_stdstar.c \
                        irplib_plugin.c \
                        irplib_polynomial.c \
                        irplib_ppm.c \
                        irplib_spectrum.c \
                        irplib_strehl.c \
                        irplib_hist.c \
                        irplib_wavecal.c \
                        irplib_wcs.c \
                        irplib_wlxcorr.c \
                        irplib_mkmaster.c \
                        irplib_ksigma_clip.c \
                        irplib_sdp_spectrum.c

libirplib_la_LDFLAGS = $(CPL_LDFLAGS) $(GSL_LDFLAGS)
libirplib_la_LIBADD = $(LIBCPLDFS) $(LIBCPLUI) $(LIBCPLDRS) $(LIBCPLCORE) $(GSL_LIBS)
libirplib_la_DEPENDENCIES =
