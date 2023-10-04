## Process this file with automake to produce Makefile.in

##   This file is part of the METIS Pipeline Library
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
AM_LDFLAGS = $(CPL_LDFLAGS) $(HDRL_LDFLAGS)
LDADD = $(LIBMETIS) $(HDRL_LIBS) $(LIBCPLDFS) $(LIBCPLUI) $(LIBCPLDRS) $(LIBCPLCORE)

check_PROGRAMS = metis_dfs-test metis_pfits-test

metis_dfs_test_SOURCES = metis_dfs-test.c
metis_pfits_test_SOURCES = metis_pfits-test.c

# Be sure to reexport important environment variables.
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
        CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
        LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
        OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
        MALLOC_PERTURB_=31 MALLOC_CHECK_=2

TESTS = $(check_PROGRAMS)

# Expected failures
XFAIL_TESTS =

# We need to remove any files that the above tests created.
clean-local:

