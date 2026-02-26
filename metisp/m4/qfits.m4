
# CPL_CHECK_QFITS
#----------------
# Checks for the Qfits library and header files.
AC_DEFUN([CPL_CHECK_QFITS],
[

    AC_MSG_CHECKING([for Qfits])

    cpl_qfits_check_header="qfits.h"
    cpl_qfits_check_lib="libqfits.a"

    cpl_qfits_includes=""
    cpl_qfits_libraries=""

    AC_ARG_WITH(qfits,
                AS_HELP_STRING([--with-qfits],
                               [location where Qfits is installed]),
                [
                    cpl_with_qfits_includes=$withval/include
                    cpl_with_qfits_libs=$withval/lib
                ])

    AC_ARG_WITH(qfits-includes,
                AS_HELP_STRING([--with-qfits-includes],
                               [location of the Qfits header files]),
                cpl_with_qfits_includes=$withval)

    AC_ARG_WITH(qfits-libs,
                AS_HELP_STRING([--with-qfits-libs],
                               [location of the Qfits library]),
                cpl_with_qfits_libs=$withval)

    AC_ARG_ENABLE(qfits-test,
                  AS_HELP_STRING([--disable-qfits-test],
                                 [disables checks for the Qfits library and headers]),
                  cpl_enable_qfits_test=$enableval,
                  cpl_enable_qfits_test=yes)


    if test "x$cpl_enable_qfits_test" = xyes; then

        # Check for the Qfits includes

        if test -z "$cpl_with_qfits_includes"; then
            cpl_qfits_incdirs="/opt/qfits/include /usr/local/include /usr/include"

            test -n "$CPLDIR" && cpl_qfits_incdirs="$CPLDIR/include \
                                                    $cpl_qfits_incdirs"

            test -n "$QFITSDIR" && cpl_qfits_incdirs="$QFITSDIR/include \
                                                      $cpl_qfits_incdirs"
        else
            cpl_qfits_incdirs="$cpl_with_qfits_includes"
        fi

        ESO_FIND_FILE($cpl_qfits_check_header, $cpl_qfits_incdirs,
                      cpl_qfits_includes)


        # Check for the Qfits library

        if test -z "$cpl_with_qfits_libs"; then
            cpl_qfits_libdirs="/opt/qfits/lib /usr/local/lib /usr/lib"

            test -n "$CPLDIR" && cpl_qfits_libdirs="$CPLDIR/lib \
                                                    $cpl_qfits_libdirs"

            test -n "$QFITSDIR" && cpl_qfits_libdirs="$QFITSDIR/lib \
                                                      $cpl_qfits_libdirs"
        else
            cpl_qfits_libdirs="$cpl_with_qfits_libs"
        fi

        ESO_FIND_FILE($cpl_qfits_check_lib, $cpl_qfits_libdirs,
                      cpl_qfits_libraries)


        if test x"$cpl_qfits_includes" = xno || \
            test x"$cpl_qfits_libraries" = xno; then
            cpl_qfits_notfound=""

            if test x"$cpl_qfits_includes" = xno; then
                if test x"$cpl_qfits_libraries" = xno; then
                    cpl_qfits_notfound="(headers and libraries)"
                else            
                    cpl_qfits_notfound="(headers)"
                fi
            else
                cpl_qfits_notfound="(libraries)"
            fi

            AC_MSG_ERROR([Qfits $cpl_qfits_notfound was not found on your system. Please check!])
        else
            AC_MSG_RESULT([libraries $cpl_qfits_libraries, headers $cpl_qfits_includes])
        fi

        # Set up the symbols

        QFITS_INCLUDES="-I$cpl_qfits_includes"
        QFITS_LDFLAGS="-L$cpl_qfits_libraries"
        LIBQFITS="-lqfits"
    else
        AC_MSG_RESULT([disabled])
        AC_MSG_WARN([Qfits checks have been disabled! This package may not build!])
        QFITS_INCLUDES=""
        QFITS_LDFLAGS=""
        LIBQFITS=""
    fi

    AC_SUBST(QFITS_INCLUDES)
    AC_SUBST(QFITS_LDFLAGS)
    AC_SUBST(LIBQFITS)

    # Check whether we are using Qfits 4.3.5 or a later version

    AC_CACHE_CHECK([whether Qfits provides qfits_get_datetime_iso8601],
                   [cpl_cv_func_qfits_get_datetime_iso8601],
                   [
                       AC_LANG_PUSH(C)

                       cpl_cppflags_save="$CPPFLAGS"
                       cpl_cflags_save="$CFLAGS"
                       cpl_ldflags_save="$LDFLAGS"
                       cpl_libs_save="$LIBS"

                       if test x$GCC = xyes; then
                           CFLAGS="$CFLAGS -pedantic-errors"
                           CPPFLAGS="$CPPFLAGS $CFLAGS"
                       fi

                       CFLAGS="$CFLAGS $QFITS_INCLUDES"
                       LDFLAGS="$LDFLAGS $QFITS_LDFLAGS"
                       LIBS="$LIBS $LIBQFITS"

                       AC_RUN_IFELSE([
#include <qfits.h>
int
main(void)
{

    const char *s = NULL;

    s = qfits_get_datetime_iso8601();
    return 0;

}
                                     ],
                                     cpl_cv_func_qfits_get_datetime_iso8601=yes,
                                     cpl_cv_func_qfits_get_datetime_iso8601=no,
                                     cpl_cv_func_qfits_get_datetime_iso8601=no)


                       CPPFLAGS="$cpl_cppflags_save"
                       CFLAGS="$cpl_cflags_save"
                       LDFLAGS="$cpl_ldflags_save"
                       LIBS="$cpl_libs_save"

                       AC_LANG_POP(C)

                   ])

    AH_TEMPLATE([HAVE_QFITS_GET_DATETIME_ISO8601],
                [Define if Qfits provides the `qfits_get_datetime_iso8601' 
                 function])

    if test x$cpl_cv_func_qfits_get_datetime_iso8601 = xyes; then
        AC_DEFINE(HAVE_QFITS_GET_DATETIME_ISO8601)
    fi

])

