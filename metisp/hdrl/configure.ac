# Process this file with autoconf to produce a configure script.

AC_INIT([HDRL Pipeline],[1.6.0],[https://support.eso.org],[hdrl])
AC_PREREQ([2.59])

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_AUX_DIR([admin])
AC_CONFIG_MACRO_DIR([m4macros])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

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

# Order of arguments: VERSION, CURRENT, REVISION, AGE
HDRL_SET_VERSION_INFO([$VERSION], [1], [0], [0])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AM_PROG_CC_C_O
AC_PROG_LN_S

AC_SYS_LARGEFILE

ESO_ENABLE_DEBUG(no)
ESO_ENABLE_STRICT(no)

ESO_PROG_CC_FLAG([Wabsolute-value                ], [CFLAGS="$CFLAGS  -Wabsolute-value       "])
ESO_PROG_CC_FLAG([Wall                           ], [CFLAGS="-Wall $CFLAGS                   "])
ESO_PROG_CC_FLAG([Wdiscarded-qualifiers          ], [CFLAGS="$CFLAGS  -Wdiscarded-qualifiers "])
ESO_PROG_CC_FLAG([Wformat-overflow               ], [CFLAGS="$CFLAGS  -Wformat-overflow      "])
ESO_PROG_CC_FLAG([Wformat-truncation             ], [CFLAGS="$CFLAGS  -Wformat-truncation    "])
ESO_PROG_CC_FLAG([Wimplicit-fallthrough          ], [CFLAGS="$CFLAGS  -Wimplicit-fallthrough "])
ESO_PROG_CC_FLAG([Wmaybe-uninitialized           ], [CFLAGS="$CFLAGS  -Wmaybe-uninitialized  "])
ESO_PROG_CC_FLAG([Wmissing-field-initializers    ], [CFLAGS="$CFLAGS  -Wmissing-field-initializers    "])
ESO_PROG_CC_FLAG([Wshadow                        ], [CFLAGS="$CFLAGS  -Wshadow               "])
ESO_PROG_CC_FLAG([Wstringop-truncation           ], [CFLAGS="$CFLAGS  -Wstringop-truncation  "])
ESO_PROG_CC_FLAG([Wunused-function               ], [CFLAGS="$CFLAGS  -Wunused-function      "])
ESO_PROG_CC_FLAG([Wunused-parameter              ], [CFLAGS="$CFLAGS  -Wunused-parameter     "])
ESO_PROG_CC_FLAG([Wunused-variable               ], [CFLAGS="$CFLAGS  -Wunused-variable      "])
ESO_PROG_CC_FLAG([fno-common                     ], [CFLAGS="$CFLAGS  -fno-common            "])
ESO_PROG_CC_FLAG([fstrict-aliasing               ], [CFLAGS="$CFLAGS  -fstrict-aliasing      "])
ESO_PROG_CC_FLAG([fvisibility=hidden             ], [CFLAGS="-fvisibility=hidden $CFLAGS     "])
ESO_PROG_CC_FLAG([pthread                        ], [CFLAGS="$CFLAGS -pthread                "])
ESO_PROG_CC_FLAG([std=c99                        ], [CFLAGS="$CFLAGS  -std=c99               "])
ESO_PROG_CC_FLAG([Wold-style-definition          ], [CFLAGS="$CFLAGS  -Wold-style-definition "])

ESO_PROG_CC_FLAG([Werror=implicit-function-declaration], [CFLAGS="-Werror=implicit-function-declaration $CFLAGS"])
ESO_PROG_CC_FLAG([Werror=incompatible-pointer-types], [CFLAGS="$CFLAGS -Werror=incompatible-pointer-types"])


# simple openmp check, the one in eso.m4 is unnecessarily fragile
# ESO_ENABLE_OPENMP(openmp=no)
#-----------------------------
AC_DEFUN([ESO_ENABLE_OPENMP],
[
    AC_REQUIRE([AC_PROG_CC])

    AC_ARG_ENABLE(openmp,
                  AS_HELP_STRING([--enable-openmp],
                                 [compiles with openmp compiler options (may not work!) [[default=$1]]]),
                  eso_enable_openmp=$enableval, eso_enable_openmp=$1)

    AC_CACHE_CHECK([whether openmp compiler options should be used],
                   eso_cv_enable_openmp,
                   eso_cv_enable_openmp=$eso_enable_openmp)


    if test x"$eso_cv_enable_openmp" = xyes; then
        ESO_PROG_CC_FLAG([fopenmp], [CFLAGS="$CFLAGS -fopenmp"
                                     LDFLAGS="$LDFLAGS -fopenmp"])
        ESO_PROG_CC_FLAG([xopenmp], [CFLAGS="$CFLAGS -xopenmp"
                                     LDFLAGS="$LDFLAGS -xopenmp"])
    fi
])
ESO_ENABLE_OPENMP([yes])
ESO_ENABLE_TSAN([no])

AC_DEFUN([ESO_ENABLE_STANDALONE],
[
    AC_ARG_ENABLE(standalone,
                  AS_HELP_STRING([--enable-standalone],
                                 [enable standalone installation for VLTSW [[default=$1]]]),
                  eso_enable_standalone=$enableval, eso_enable_standalone=$1)

    AM_CONDITIONAL([VLTSW], [test x"$eso_enable_standalone" = xyes])
])

ESO_ENABLE_STANDALONE([no])

ESO_CHECK_DOCTOOLS

AC_ENABLE_STATIC(no)
AC_ENABLE_SHARED(yes)

LT_INIT
AC_SUBST(LIBTOOL_DEPS)

# Checks for header files.
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_FUNCS([floor pow sqrt])
ESO_FUNC_STRDUP

# Check for CPL presence and usability
ESO_CHECK_CPL

# Check for GSL
GSL_CHECK_LIBS()

#Check for ERFA
ESO_CHECK_ERFA

# Check for libcurl presence and usability
ESO_CHECK_LIBCURL


HDRL_CREATE_SYMBOLS

AM_WITH_DMALLOC

#AC_CONFIG_SUBDIRS(external/fftw external/sextractor external/swarp)
#AC_CONFIG_SUBDIRS(external/sextractor external/swarp)

AC_CONFIG_FILES(Makefile
                tests/Makefile
                catalogue/Makefile
                catalogue/tests/Makefile)
AC_OUTPUT
