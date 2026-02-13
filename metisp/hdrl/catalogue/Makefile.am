## Process this file with automake to produce Makefile.in

##   This file is part of the HDRL Pipeline
##   Copyright (C) 2012,2013 European Southern Observatory

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

# NOTE: Only build this directory. The tests subdir is built from the top level
# Makefile to maintain the correct build order and break the circular dependency
# between libcatalogue, libhdrl and the unit test binaries.
SUBDIRS = .

if MAINTAINER_MODE

MAINTAINERCLEANFILES = Makefile.in

endif

AM_CPPFLAGS = -DCX_LOG_DOMAIN=\"HdrlLib\"  -DHDRL_USE_PRIVATE $(CPL_INCLUDES) -I$(top_srcdir)

noinst_LTLIBRARIES = libcatalogue.la

noinst_HEADERS = \
		    hdrl_cat_def.h \
		    hdrl_cat_apclust.h \
		    hdrl_cat_apio.h \
		    hdrl_cat_apline.h \
		    hdrl_cat_areals.h \
		    hdrl_cat_casu.h \
		    hdrl_cat_conf.h \
		    hdrl_cat_utils_sort.h \
		    hdrl_cat_utils.h \
		    hdrl_cat_classify.h \
		    hdrl_cat_terminate.h \
		    hdrl_cat_background.h \
		    hdrl_cat_extend.h \
		    hdrl_cat_overlp.h \
		    hdrl_cat_phopt.h \
		    hdrl_cat_radii.h \
		    hdrl_cat_filter.h \
		    hdrl_cat_statistics.h \
		    hdrl_cat_table.h \
		    hdrl_cat_moments.h \
		    hdrl_cat_solve.h \
		    hdrl_cat_polynm.h \
		    hdrl_cat_seeing.h

libcatalogue_la_SOURCES = \
		    hdrl_cat_apclust.c \
		    hdrl_cat_apio.c \
		    hdrl_cat_apline.c \
		    hdrl_cat_areals.c \
		    hdrl_cat_casu.c \
		    hdrl_cat_conf.c \
		    hdrl_cat_utils_sort.c \
		    hdrl_cat_utils.c \
		    hdrl_cat_classify.c \
		    hdrl_cat_terminate.c \
		    hdrl_cat_background.c \
		    hdrl_cat_extend.c \
		    hdrl_cat_overlp.c \
		    hdrl_cat_phopt.c \
		    hdrl_cat_radii.c \
		    hdrl_cat_filter.c \
		    hdrl_cat_statistics.c \
		    hdrl_cat_table.c \
		    hdrl_cat_moments.c \
		    hdrl_cat_solve.c \
		    hdrl_cat_polynm.c \
		    hdrl_cat_seeing.c
