# on SL6 aclocal 1.11 doesn't check this folder even though its in
# AC_CONFIG_MACRO_DIR. Fixed in later aclocal versions
m4_include([m4macros/cpl-pipeline.m4])
m4_include([m4macros/eso.m4])

# HDRL_SET_VERSION_INFO(VERSION, [CURRENT], [REVISION], [AGE])
#----------------------------------------------------------------
# Setup various version information, especially the libtool versioning
AC_DEFUN([HDRL_SET_VERSION_INFO],
[
    hdrldemo_version=`echo "$1" | sed -e 's/[[a-z,A-Z]].*$//'`

    hdrldemo_major_version=`echo "$hdrldemo_version" | \
        sed 's/\([[0-9]]*\).\(.*\)/\1/'`
    hdrldemo_minor_version=`echo "$hdrldemo_version" | \
        sed 's/\([[0-9]]*\).\([[0-9]]*\)\(.*\)/\2/'`
    hdrldemo_micro_version=`echo "$hdrldemo_version" | \
        sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    if test -z "$hdrldemo_major_version"; then hdrldemo_major_version=0
    fi

    if test -z "$hdrldemo_minor_version"; then hdrldemo_minor_version=0
    fi

    if test -z "$hdrldemo_micro_version"; then hdrldemo_micro_version=0
    fi

    HDRL_VERSION="$hdrldemo_version"
    HDRL_MAJOR_VERSION=$hdrldemo_major_version
    HDRL_MINOR_VERSION=$hdrldemo_minor_version
    HDRL_MICRO_VERSION=$hdrldemo_micro_version

    if test -z "$4"; then HDRL_INTERFACE_AGE=0
    else HDRL_INTERFACE_AGE="$4"
    fi

    HDRL_BINARY_AGE=`expr 100 '*' $HDRL_MINOR_VERSION + $HDRL_MICRO_VERSION`
    HDRL_BINARY_VERSION=`expr 10000 '*' $HDRL_MAJOR_VERSION + \
                          $HDRL_BINARY_AGE`

    AC_SUBST(HDRL_VERSION)
    AC_SUBST(HDRL_MAJOR_VERSION)
    AC_SUBST(HDRL_MINOR_VERSION)
    AC_SUBST(HDRL_MICRO_VERSION)
    AC_SUBST(HDRL_INTERFACE_AGE)
    AC_SUBST(HDRL_BINARY_VERSION)
    AC_SUBST(HDRL_BINARY_AGE)

    AC_DEFINE_UNQUOTED(HDRL_MAJOR_VERSION, $HDRL_MAJOR_VERSION,
                       [HDRL major version number])
    AC_DEFINE_UNQUOTED(HDRL_MINOR_VERSION, $HDRL_MINOR_VERSION,
                       [HDRL minor version number])
    AC_DEFINE_UNQUOTED(HDRL_MICRO_VERSION, $HDRL_MICRO_VERSION,
                       [HDRL micro version number])
    AC_DEFINE_UNQUOTED(HDRL_INTERFACE_AGE, $HDRL_INTERFACE_AGE,
                       [HDRL interface age])
    AC_DEFINE_UNQUOTED(HDRL_BINARY_VERSION, $HDRL_BINARY_VERSION,
                       [HDRL binary version number])
    AC_DEFINE_UNQUOTED(HDRL_BINARY_AGE, $HDRL_BINARY_AGE,
                       [HDRL binary age])

    ESO_SET_LIBRARY_VERSION([$2], [$3], [$4])
])


# HDRL_CREATE_SYMBOLS
#-----------------------
# Define include and library related makefile symbols
AC_DEFUN([HDRL_CREATE_SYMBOLS],
[

    # Symbols for package include file and library search paths

    HDRL_INCLUDES='-I$(top_srcdir)'
    HDRL_LDFLAGS='-L$(top_builddir)'

    # Library aliases

    LIBHDRL='$(top_builddir)/libhdrl.la'

    # Substitute the defined symbols

    AC_SUBST(HDRL_INCLUDES)
    AC_SUBST(HDRL_LDFLAGS)

    AC_SUBST(LIBHDRL)
    AC_SUBST(LIBHDRL)

    # Check for CPL and user defined libraries
    AC_REQUIRE([ESO_CHECK_CPL])

    all_includes='$(HDRL_INCLUDES) $(CPL_INCLUDES)'
    all_ldflags='$(HDRL_LDFLAGS) $(CPL_LDFLAGS)'

    AC_SUBST(all_includes)
    AC_SUBST(all_ldflags)
])
