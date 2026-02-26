## Process this file with automake to produce Makefile.in

## This file is part of the METIS Pipeline
## Copyright (C) 2002-2017 European Southern Observatory

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

AUTOMAKE_OPTIONS = 1.8 foreign

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = *~


SUBDIRS = irplib hdrl metis recipes pymetis pyrecipes reflex workflows regtests
DIST_SUBDIRS = irplib hdrl metis recipes pymetis pyrecipes reflex workflows regtests

DOXYGEN_SUBDIRS =
PYTHON_SUBDIRS = pymetis

EXTRA_DIST = BUGS Doxyfile.in admin/doxygen.am admin/python.am


pipedocs_DATA = ChangeLog AUTHORS NEWS README.md TODO COPYING


if MAINTAINER_MODE

DISTCHECK_CONFIGURE_FLAGS = --enable-maintainer-mode

MAINTAINERCLEANFILES = $(top_srcdir)/Makefile.in \
    $(top_srcdir)/aclocal.m4 $(top_srcdir)/configure \
    $(top_srcdir)/config.h.in $(top_srcdir)/config.h.in~ \
    $(top_srcdir)/admin/compile $(top_srcdir)/admin/config.guess \
    $(top_srcdir)/admin/config.sub $(top_srcdir)/admin/depcomp \
    $(top_srcdir)/admin/install-sh $(top_srcdir)/admin/ltmain.sh \
    $(top_srcdir)/admin/missing $(top_srcdir)/admin/test-driver \
    $(top_builddir)/config.log $(top_builddir)/config.status

clean-local: clean-doxygen

dist-hook: doxygen
	@if test -d $(top_builddir)/html; then \
	  echo "cp -pr $(top_builddir)/html $(distdir)"; \
	  cp -pr $(top_builddir)/html $(distdir); \
	fi
	find $(distdir) -type d ! -perm -222 -exec chmod u+w {} \; -o \
	-type f ! -perm -222 -exec chmod u+w {} \; || chmod -R u+w $(distdir)

else

clean-local:

endif

uninstall-local: uninstall-doxygen

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

include $(top_srcdir)/admin/doxygen.am
include $(top_srcdir)/admin/python.am
