# LIB_CHECK_GSL
#------------------
# Checks for the gsl library and header files.
AC_DEFUN([CPL_CHECK_GSL],
[

    AC_MSG_CHECKING([for gsl])

    AC_ARG_WITH(gsl,
                AS_HELP_STRING([--with-gsl],
                               [location where gsl is installed]),
                [
                    cpl_gsl_dir=$withval
                ])

    export PKG_CONFIG_PATH="$cpl_gsl_dir/lib/pkgconfig:$cpl_gsl_dir/lib64/pkgconfig:$GSLDIR/lib/pkgconfig/:$GSLDIR/lib64/pkgconfig/:$PKG_CONFIG_PATH"

    AC_SUBST(PKG_CONFIG_PATH)
    PKG_CHECK_MODULES([GSL], [gsl], 
                      [AC_DEFINE([HAVE_LIBGSL], [1], [Define to 1 iff you have GSL])
                       AC_DEFINE(HAVE_GSL, 1, [Define to 1 iff you have GSL])],
                      AC_MSG_WARN([No GSL available]))

])

# GSL_CHECK_LIBS
#---------------
# Checks for the GSL libraries and header files, does not require pkg-config
# If the first argument is "optional" the check is allowed to fail, otherwise
# missing GSL is a fatal error
# adds --with-gsl{,-libs,-includes}, --disable-gsl-test and sets
# GSL_INCLUDES
# GSL_LDFLAGS
# GSL_LIBS
# It defines HAVE_GSL and HAVE_LIBGSL in config.h
AC_DEFUN([GSL_CHECK_LIBS],
[

    AC_MSG_CHECKING([for GSL])

    gsl_check_gsl_lib="-lgsl -lgslcblas"

    gsl_includes=""
    gsl_libraries=""

    AC_ARG_WITH(gsl,
                AS_HELP_STRING([--with-gsl],
                               [location where GSL is installed]),
                [
                    gsl_with_gsl_includes=$withval/include
                    # opensuse defaults also non systems installations to lib64 ...
                    gsl_with_gsl_libs="-L$withval/lib -L$withval/lib64 -L$withval/lib32"
                ])

    AC_ARG_WITH(gsl-includes,
                AS_HELP_STRING([--with-gsl-includes],
                               [location of the GSL header files]),
                gsl_with_gsl_includes=$withval)

    AC_ARG_WITH(gsl-libs,
                AS_HELP_STRING([--with-gsl-libs],
                               [location of the GSL library]),
                gsl_with_gsl_libs=-L$withval)

    AC_ARG_ENABLE(gsl-test,
                  AS_HELP_STRING([--disable-gsl-test],
                                 [disables checks for the GSL library and headers]),
                  gsl_enable_gsl_test=$enableval,
                  gsl_enable_gsl_test=yes)


    if test "x$gsl_enable_gsl_test" = xyes; then
        save_LDFLAGS="$LDFLAGS"
        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        if test -n "$gsl_with_gsl_libs"; then
            LDFLAGS="$gsl_with_gsl_libs"
        elif test -n "$GSLDIR"; then
            LDFLAGS="-L$GSLDIR/lib -L$GSLDIR/lib64 -L$GSLDIR/lib32"
        else
            LDFLAGS=""
        fi
        if test -n "$gsl_with_gsl_includes"; then
            CFLAGS="-I$gsl_with_gsl_includes"
        elif test -n "$GSLDIR"; then
            CFLAGS="-I$GSLDIR/include"
        else
            CFLAGS=""
        fi
        LIBS=$gsl_check_gsl_lib

        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <gsl/gsl_sf_bessel.h>]], [[gsl_sf_bessel_J0(5.);]])],[
                        AC_MSG_RESULT(yes)
                        AC_DEFINE(HAVE_GSL, 1, [Define to 1 iff you have GSL])
                        AC_DEFINE(HAVE_LIBGSL, 1, [Define to 1 iff you have GSL])
                        # Set up the symbols
                        GSL_INCLUDES="$CFLAGS"
                        GSL_LDFLAGS="$LDFLAGS"
                        GSL_LIBS="$gsl_check_gsl_lib"
                    ],[
                        if test "x$1" = xoptional; then
                            AC_MSG_NOTICE([Optional GSL not found])
                        else
                            AC_MSG_ERROR([GSL not found])
                        fi
                    ])

        LDFLAGS="$save_LDFLAGS"
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
    else
        AC_MSG_RESULT([disabled])
        AC_MSG_WARN([GSL checks have been disabled! This package may not build!])
        GSL_INCLUDES=""
        GSL_LDFLAGS=""
        GSL_LIBS=""
    fi

    AC_SUBST(GSL_INCLUDES)
    AC_SUBST(GSL_LDFLAGS)
    AC_SUBST(GSL_LIBS)

])
