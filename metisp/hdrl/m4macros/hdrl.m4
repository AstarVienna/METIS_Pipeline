# HDRL_CHECK([location])
# ----------------------
# Sets build variables required to build and link hdrl
# the argument defines the relative location of the hdrl external in the source
# tree
# Note that this macro calls GSL_CHECK_LIBS([optional]) so additional calls to
# this macro with optional argument can be skipped in the pipeline.
AC_DEFUN([HDRL_CHECK],
[
    HDRL_LOCATION="$1"

    # check for optional GSL, needed in pipelines to correctly setup
    # HDRL_LDFLAGS and HDRL_LIBS and in the case of no GSL being present to not
    # declare functions using GSL in the hdrl headers as this requires a
    # HAVE_GSL defined in the pipelines config.h
    GSL_CHECK_LIBS([])

    #Check for ERFA
    ESO_CHECK_ERFA

    # Check for libcurl presence and usability
    ESO_CHECK_LIBCURL

    AC_MSG_CHECKING([HDRL in $HDRL_LOCATION])

    # requires cpl and libm as it is a static library
    HDRL_LIBS="-lhdrl -lcplcore -lcpldrs -lcplui -lcpldfs -lcext -lm $GSL_LIBS $LIBCURL_LIBS $ERFA_LIBS"
    # la file to be added to DEPENDENCIES libtool doesn't track static libs
    LIBHDRL="\$(top_builddir)/$HDRL_LOCATION/libhdrl.la"
    HDRL_LDFLAGS="-L\$(top_builddir)/$HDRL_LOCATION $GSL_LDFLAGS $ERFA_LDFLAGS $LIBCURL_LDFLAGS"
    HDRL_INCLUDES="-I\$(top_srcdir)/$HDRL_LOCATION"

    AC_SUBST(HDRL_LIBS)
    AC_SUBST(LIBHDRL)
    AC_SUBST(HDRL_LDFLAGS)
    AC_SUBST(HDRL_INTERNAL_CFLAGS)
    AC_SUBST(HDRL_INCLUDES)

    AC_MSG_RESULT([ok])
])
