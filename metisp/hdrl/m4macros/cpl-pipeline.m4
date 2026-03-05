# ESO_CHECK_CPL
#---------------
# Checks for the CPL libraries and header files. Intended for use by pipelines
# Sets:
# CPL_INCLUDES
# CPL_LDFLAGS
# LIBCPLCORE
# LIBCPLDRS
# LIBCPLUI
# LIBCPLDFS
# LIBCEXT
AC_DEFUN([ESO_CHECK_CPL],
[

    AC_MSG_CHECKING([for CPL])

    # assumes if one is there all are there as there is no way to do a partial
    # install with cpl
    cpl_check_cpl_lib="-lcplcore"

    cpl_includes=""
    cpl_ldflags=""

    AC_ARG_WITH(cpl,
                AS_HELP_STRING([--with-cpl],
                               [location where CPL is installed]),
                [
                    cpl_with_cpl_includes=$withval/include
                    # opensuse defaults also non systems installations to lib64 ...
                    cpl_with_cpl_libs="-L$withval/lib -L$withval/lib64 -L$withval/lib32"
                ])

    AC_ARG_WITH(cpl-includes,
                AS_HELP_STRING([--with-cpl-includes],
                               [location of the CPL header files]),
                cpl_with_cpl_includes=$withval)

    AC_ARG_WITH(cpl-libs,
                AS_HELP_STRING([--with-cpl-libs],
                               [location of the CPL library]),
                cpl_with_cpl_libs=-L$withval)

    AC_ARG_ENABLE(cpl-test,
                  AS_HELP_STRING([--disable-cpl-test],
                                 [disables checks for the CPL library and headers]),
                  cpl_enable_cpl_test=$enableval,
                  cpl_enable_cpl_test=yes)


    if test "x$cpl_enable_cpl_test" = xyes; then

        AC_MSG_CHECKING(checking for cpl)
        save_LDFLAGS="$LDFLAGS"
        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        if test -n "$cpl_with_cpl_libs"; then
            LDFLAGS="$cpl_with_cpl_libs"
        elif test -n "$CPLDIR"; then
            LDFLAGS="-L$CPLDIR/lib -L$CPLDIR/lib64 -L$CPLDIR/lib32"
        else
            LDFLAGS=""
        fi
        if test -n "$cpl_with_cpl_includes"; then
            CFLAGS="-I$cpl_with_cpl_includes"
        elif test -n "$CPLDIR"; then
            CFLAGS="-I$CPLDIR/include"
        else
            CFLAGS=""
        fi
        LIBS=$cpl_check_cpl_lib

        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <cpl.h>]], [[cpl_init(0);]])],[
                        AC_MSG_RESULT(yes)
                        cpl_includes=$CFLAGS
                        cpl_ldflags=$LDFLAGS
                    ],[
                        AC_MSG_ERROR([CPL not found])
                    ])

        # Set up the symbols

        CPL_INCLUDES="$cpl_includes"
        CPL_LDFLAGS="$cpl_ldflags"
        LDFLAGS="$save_LDFLAGS"
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"

        LIBCPLCORE='-lcplcore'
        LIBCPLDRS='-lcpldrs'
        LIBCPLUI='-lcplui'
        LIBCPLDFS='-lcpldfs'
        LIBCEXT='-lcext'
    else
        AC_MSG_RESULT([disabled])
        AC_MSG_WARN([CPL checks have been disabled! This package may not build!])
        CPL_INCLUDES=""
        CPL_LDFLAGS=""
        LIBCPLCORE=""
        LIBCPLDRS=""
        LIBCPLUI=""
        LIBCPLDFS=""
        LIBCEXT=""
    fi

    AC_SUBST(CPL_INCLUDES)
    AC_SUBST(CPL_LDFLAGS)
    AC_SUBST(LIBCPLCORE)
    AC_SUBST(LIBCPLDRS)
    AC_SUBST(LIBCPLUI)
    AC_SUBST(LIBCPLDFS)
    AC_SUBST(LIBCEXT)
])
