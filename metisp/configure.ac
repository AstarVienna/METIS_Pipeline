# Process this file with autoconf to produce a configure script.

AC_INIT([METIS Instrument Pipeline], [0.0.1], [https://support.eso.org],
[metis])
AC_PREREQ([2.59])

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_AUX_DIR([admin])
AC_CONFIG_SUBDIRS([hdrl])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE([enable])

# Immediately before every release do:
#-------------------------------------
#   if (the interface is totally unchanged from previous release)
#       REVISION++;
#   else {
#       /* interfaces have been added, removed or changed */
#       REVISION = 0;
#       CURRENT++;
#       if (any interfaces have been _added_ since last release)
#           AGE++;
#       if (any interfaces have been _removed_ or incompatibly changed)
#           AGE = 0;
#   }

METIS_SET_VERSION_INFO([$VERSION])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AM_PROG_CC_C_O
AC_PROG_LN_S

# large file support
AC_SYS_LARGEFILE

ESO_ENABLE_DEBUG(no)
ESO_ENABLE_STRICT(no)

AC_PROG_CC_C99
AC_DEFINE([POSIX_C_SOURCE], [200809L], [Minimun version of POSIX standard to use. Needed by mkdtemp])
AC_DEFINE([XOPEN_SOURCE], [700], [Minimun version of SUS standard to use])
ESO_PROG_CC_FLAG([Werror=implicit-function-declaration], [CFLAGS="$CFLAGS -Werror=implicit-function-declaration"])
ESO_PROG_CC_FLAG([Werror=incompatible-pointer-types], [CFLAGS="$CFLAGS -Werror=incompatible-pointer-types"])

ESO_CHECK_DOCTOOLS

AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)

AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.
AC_CHECK_LIB(m, pow, [LIBS="$LIBS -lm"])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Check for CPL presence and usability
CPL_CHECK_LIBS

# Check for <xx>clipm
# - can be called multiple times
# - replace the argument "xxclipm" by your library name,
#   e.g. with KMOS: XXCLIPM_CHECK_LIB(kmclipm)
# Comment in if required!
# XXCLIPM_CHECK_LIB(xxclipm)

# setup hdrl folder
HDRL_CHECK([hdrl])

# call after CPL_CHECK_LIBS and XXCLIPM_CHECK_LIB
METIS_SET_PATHS
METIS_CREATE_SYMBOLS

AC_CONFIG_COMMANDS(pymetis/setup.cfg, [
                    cd pymetis && cp -a setup.cfg $ac_srcdir/setup.cfg
                ])
AC_CONFIG_FILES(Makefile
                Doxyfile
                metis/Makefile
                metis/tests/Makefile
                recipes/Makefile
                pymetis/Makefile
                pyrecipes/Makefile
                irplib/Makefile
                irplib/tests/Makefile
                workflows/Makefile
                reflex/Makefile
                reflex/metis.xml
                regtests/Makefile
                regtests/tests/Makefile)
AC_OUTPUT
