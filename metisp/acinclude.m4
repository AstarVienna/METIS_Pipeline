# METIS_SET_VERSION_INFO(VERSION, [CURRENT], [REVISION], [AGE])
#----------------------------------------------------------------
# Setup various version information, especially the libtool versioning
AC_DEFUN([METIS_SET_VERSION_INFO],
[
    metis_version=`echo "$1" | sed -e 's/[[a-z,A-Z]].*$//'`

    metis_major_version=`echo "$metis_version" | \
        sed 's/\([[0-9]]*\).\(.*\)/\1/'`
    metis_minor_version=`echo "$metis_version" | \
        sed 's/\([[0-9]]*\).\([[0-9]]*\)\(.*\)/\2/'`
    metis_micro_version=`echo "$metis_version" | \
        sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    if test -z "$metis_major_version"; then metis_major_version=0
    fi

    if test -z "$metis_minor_version"; then metis_minor_version=0
    fi

    if test -z "$metis_micro_version"; then metis_micro_version=0
    fi

    METIS_VERSION="$metis_version"
    METIS_MAJOR_VERSION=$metis_major_version
    METIS_MINOR_VERSION=$metis_minor_version
    METIS_MICRO_VERSION=$metis_micro_version

    if test -z "$4"; then METIS_INTERFACE_AGE=0
    else METIS_INTERFACE_AGE="$4"
    fi

    METIS_BINARY_AGE=`expr 100 '*' $METIS_MINOR_VERSION + $METIS_MICRO_VERSION`
    METIS_BINARY_VERSION=`expr 10000 '*' $METIS_MAJOR_VERSION + \
                          $METIS_BINARY_AGE`

    AC_SUBST(METIS_VERSION)
    AC_SUBST(METIS_MAJOR_VERSION)
    AC_SUBST(METIS_MINOR_VERSION)
    AC_SUBST(METIS_MICRO_VERSION)
    AC_SUBST(METIS_INTERFACE_AGE)
    AC_SUBST(METIS_BINARY_VERSION)
    AC_SUBST(METIS_BINARY_AGE)

    AC_DEFINE_UNQUOTED(METIS_MAJOR_VERSION, $METIS_MAJOR_VERSION,
                       [METIS major version number])
    AC_DEFINE_UNQUOTED(METIS_MINOR_VERSION, $METIS_MINOR_VERSION,
                       [METIS minor version number])
    AC_DEFINE_UNQUOTED(METIS_MICRO_VERSION, $METIS_MICRO_VERSION,
                       [METIS micro version number])
    AC_DEFINE_UNQUOTED(METIS_INTERFACE_AGE, $METIS_INTERFACE_AGE,
                       [METIS interface age])
    AC_DEFINE_UNQUOTED(METIS_BINARY_VERSION, $METIS_BINARY_VERSION,
                       [METIS binary version number])
    AC_DEFINE_UNQUOTED(METIS_BINARY_AGE, $METIS_BINARY_AGE,
                       [METIS binary age])

    ESO_SET_LIBRARY_VERSION([$2], [$3], [$4])
])


# METIS_SET_PATHS
#------------------
# Define auxiliary directories of the installed directory tree.
AC_DEFUN([METIS_SET_PATHS],
[

    if test -z "$plugindir"; then
        plugindir='${libdir}/esopipes-plugins/${PACKAGE}-${VERSION}'
    fi

    if test -z "$privatelibdir"; then
        privatelibdir='${libdir}/${PACKAGE}-${VERSION}'
    fi

    if test -z "$apidocdir"; then
        apidocdir='${datadir}/doc/esopipes/${PACKAGE}-${VERSION}/html'
    fi

    if test -z "$pipedocsdir"; then
        pipedocsdir='${datadir}/doc/esopipes/${PACKAGE}-${VERSION}'
    fi

    if test -z "$configdir"; then
       configdir='${datadir}/${PACKAGE}/config'
    fi

    if test -z "$wkfextradir"; then
        wkfextradir='${datadir}/esopipes/${PACKAGE}-${VERSION}/reflex'
    fi

    if test -z "$wkfcopydir"; then
        wkfcopydir='${datadir}/reflex/workflows/${PACKAGE}-${VERSION}'
    fi

    if test -z "$workflowdir"; then
        workflowdir='${datadir}/esopipes/workflows/${PACKAGE}-${VERSION}/${PACKAGE}'
    fi

    if test -z "$reportsdir"; then
       reportsdir='${datadir}/esopipes/reports/${PACKAGE}-${VERSION}'
    fi


    AC_SUBST(plugindir)
    AC_SUBST(privatelibdir)
    AC_SUBST(apidocdir)
    AC_SUBST(pipedocsdir)
    AC_SUBST(configdir)
    AC_SUBST(wkfextradir)
    AC_SUBST(workflowdir)
    AC_SUBST(wkfcopydir)


])


# METIS_CREATE_SYMBOLS
#-----------------------
# Define include and library related makefile symbols
AC_DEFUN([METIS_CREATE_SYMBOLS],
[

    # Symbols for package include file and library search paths

    METIS_INCLUDES='-I$(top_srcdir)/metis -I$(top_srcdir)/irplib'
    METIS_LDFLAGS='-L$(top_builddir)/metis'

    # Library aliases

    LIBMETIS='$(top_builddir)/metis/libmetis.la'
    LIBIRPLIB='$(top_builddir)/irplib/libirplib.la'

    # Substitute the defined symbols

    AC_SUBST(METIS_INCLUDES)
    AC_SUBST(METIS_LDFLAGS)

    AC_SUBST(LIBMETIS)
    AC_SUBST(LIBIRPLIB)

    # Check for CPL and user defined libraries
    AC_REQUIRE([CPL_CHECK_LIBS])
    AC_REQUIRE([ESO_CHECK_EXTRA_LIBS])

    all_includes='$(METIS_INCLUDES) $(CPL_INCLUDES) $(EXTRA_INCLUDES) $(XXCLIPM_INCLUDES)'
    all_ldflags='$(METIS_LDFLAGS) $(CPL_LDFLAGS) $(EXTRA_LDFLAGS) $(XXCLIPM_LDFLAGS)'

    AC_SUBST(all_includes)
    AC_SUBST(all_ldflags)
])
