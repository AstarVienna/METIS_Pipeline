# ESO_CHECK_ERFA
#------------------
# Checks for the erfa library and header files.
AC_DEFUN([ESO_CHECK_ERFA],
[

    AC_ARG_WITH(erfa,
                AS_HELP_STRING([--with-erfa],
                               [location where erfa is installed]),
                [
                    erfa_dir=$withval
                ])

    export PKG_CONFIG_PATH="$erfa_dir/lib/pkgconfig:$erfa_dir/lib64/pkgconfig:$ERFADIR/lib/pkgconfig/:$ERFADIR/lib64/pkgconfig/:$PKG_CONFIG_PATH"

    AC_SUBST(PKG_CONFIG_PATH)
    PKG_CHECK_MODULES([ERFA], [erfa], 
                      AC_DEFINE([HAVE_LIBERFA], [1], [1 if ERFA present]),
                      AC_MSG_ERROR([No ERFA available]))

])
