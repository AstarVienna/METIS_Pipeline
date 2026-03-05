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

SUBDIRS = . tests

DISTCLEANFILES = *~


if MAINTAINER_MODE

MAINTAINERCLEANFILES = Makefile.in

endif


AM_CPPFLAGS = -DCX_LOG_DOMAIN=\"MetisLib\" $(all_includes)

noinst_HEADERS = metis_utils.h \
                 metis_pfits.h \
                 metis_dfs.h

pkginclude_HEADERS =

privatelib_LTLIBRARIES = libmetis.la

libmetis_la_SOURCES = metis_utils.c \
                             metis_pfits.c \
                             metis_dfs.c

libmetis_la_LDFLAGS = $(CPL_LDFLAGS) $(XXCLIPM_LDFLAGS) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
libmetis_la_LIBADD = $(LIBCPLDFS) $(LIBCPLUI) $(LIBCPLDRS) $(LIBCPLCORE) $(LIBXXCLIPM)
libmetis_la_DEPENDENCIES =
