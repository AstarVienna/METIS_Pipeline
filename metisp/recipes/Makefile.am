## Process this file with automake to produce Makefile.in

##   This file is part of the METIS Pipeline
##   Copyright (C) 2002-2017 European Southern Observatory

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
##   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AUTOMAKE_OPTIONS = 1.8 foreign

DISTCLEANFILES = *~


if MAINTAINER_MODE

MAINTAINERCLEANFILES = Makefile.in

endif


AM_CPPFLAGS = $(all_includes)
AM_LDFLAGS  = -module -avoid-version -no-undefined

noinst_HEADERS =

plugin_LTLIBRARIES = metis.la \
					 metis_calib.la

metis_la_SOURCES = metis.c
metis_la_LIBADD = $(LIBMETIS)
metis_la_DEPENDENCIES = $(LIBMETIS)

metis_calib_la_SOURCES = metis_calib.c
metis_calib_la_LIBADD = $(LIBMETIS)
metis_calib_la_DEPENDENCIES = $(LIBMETIS)

