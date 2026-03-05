# TELLURICCORR_CHECK_LIBS(version)
#---------------------------------------
# Checks for the TELLURICCORR library and header files.
AC_DEFUN([TELLURICCORR_CHECK_LIBS],
[

    telluriccorr_check_version="$1"
    telluriccorr_check_header="telluriccorr.h"
    telluriccorr_check_libraries="-ltelluriccorr"

    telluriccorr_pkgconfig="telluriccorr"
    
    
    AC_REQUIRE([ESO_PROG_PKGCONFIG])
    AC_REQUIRE([CPL_CHECK_LIBS])

    AC_ARG_WITH(telluriccorr,
                AS_HELP_STRING([--with-telluriccorr],
                               [location where telluriccorr is installed]),
                [
                    telluriccorr_with_telluriccorr=$withval
                ])

    AC_ARG_WITH(telluriccorr-includes,
                AS_HELP_STRING([--with-telluriccorr-includes],
                               [location of the telluriccorr header files]),
                telluriccorr_with_telluriccorr_includes=$withval)

    AC_ARG_WITH(telluriccorr-libs,
                AS_HELP_STRING([--with-telluriccorr-libs],
                               [location of the telluriccorr library]),
                telluriccorr_with_telluriccorr_libs=$withval)

    AC_ARG_ENABLE(telluriccorr-test,
                  AS_HELP_STRING([--disable-telluriccorr-test],
                                 [disables checks for the telluriccorr library and headers]),
                  telluriccorr_enable_telluriccorr_test=$enableval,
                  telluriccorr_enable_telluriccorr_test=yes)

    AC_ARG_VAR([TELLURICCORRDIR], [Location where telluriccorr is installed])

    if test "x$telluriccorr_enable_telluriccorr_test" = xyes; then

        AC_MSG_CHECKING([for telluriccorr])

        # If include directories and libraries are given as arguments, use them
        # initially. Otherwise assume a standard system installation of the
        # package. This may then updated in the following.
        
        telluriccorr_libs="$telluriccorr_check_libraries"
        telluriccorr_cflags="-I/usr/include/telluriccorr -I/usr/include"
        telluriccorr_ldflags=""
    
        if test -n "${PKGCONFIG}"; then
    
            $PKGCONFIG --exists $telluriccorr_pkgconfig
    
            if test x$? = x0; then
                telluriccorr_cflags="`$PKGCONFIG --cflags $telluriccorr_pkgconfig`"
                telluriccorr_ldflags="`$PKGCONFIG --libs-only-L $telluriccorr_pkgconfig`"
                telluriccorr_libs="`$PKGCONFIG --libs-only-l $telluriccorr_pkgconfig`"
            fi
        
        fi

        # Directories given as arguments replace a standard system installation
        # setup if they are given.
                
        if test -n "$TELLURICCORRDIR"; then
            telluriccorr_cflags="-I$TELLURICCORRDIR/include"
            telluriccorr_ldflags="-L$TELLURICCORRDIR/lib64 -L$TELLURICCORRDIR/lib"
        fi
        
        if test -n "$telluriccorr_with_telluriccorr"; then    
            telluriccorr_cflags="-I$telluriccorr_with_telluriccorr/include"
            telluriccorr_ldflags="-L$telluriccorr_with_telluriccorr/lib -L$telluriccorr_with_telluriccorr/lib64"
        fi    
        
        if test -n "$telluriccorr_with_telluriccorr_includes"; then
            telluriccorr_cflags="-I$telluriccorr_with_telluriccorr_includes"
        fi
        
        if test -n "$telluriccorr_with_telluriccorr_libs"; then
            telluriccorr_ldflags="-L$telluriccorr_with_telluriccorr_libs"
        fi
        
        
        # Check whether the header files and the library are present and
        # whether they can be used.
        
        telluriccorr_have_telluriccorr_libraries="no"
        telluriccorr_have_telluriccorr_headers="no"
            
        AC_LANG_PUSH(C)
        
        telluriccorr_cflags_save="$CFLAGS"
        telluriccorr_ldflags_save="$LDFLAGS"
        telluriccorr_libs_save="$LIBS"
    
        CFLAGS="$CPL_INCLUDES $telluriccorr_cflags"
        LDFLAGS="$telluriccorr_ldflags $CPL_LDFLAGS"
        LIBS="$telluriccorr_libs $LIBCPLUI $LIBCPLCORE"
    
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
                          [[
                          #include <$telluriccorr_check_header>
                          ]],
                          [
                           char c;
                          ])],
                          [
                           telluriccorr_have_telluriccorr_headers="yes"
                          ],
                          [
                           telluriccorr_have_telluriccorr_headers="no"
                          ])
    
        AC_LINK_IFELSE([AC_LANG_PROGRAM(
                       [[
		       #include <$telluriccorr_check_header>
                       ]],
                       [
                        mf_configuration_delete((void *)0);
                       ])],
                       [
                        telluriccorr_have_telluriccorr_libraries="yes"
                       ],
                       [
                        telluriccorr_have_telluriccorr_libraries="no"
                       ])
                        
        AC_LANG_POP(C)
       
        CFLAGS="$telluriccorr_cflags_save"
        LDFLAGS="$telluriccorr_ldflags_save"
        LIBS="$telluriccorr_libs_save"
        
        if test x"$telluriccorr_have_telluriccorr_libraries" = xno || \
            test x"$telluriccorr_have_telluriccorr_headers" = xno; then
            telluriccorr_notfound=""
    
            if test x"$telluriccorr_have_telluriccorr_headers" = xno; then
                if test x"$telluriccorr_have_telluriccorr_libraries" = xno; then
                    telluriccorr_notfound="(headers and libraries)"
                else
                    telluriccorr_notfound="(headers)"
                fi
            else
                telluriccorr_notfound="(libraries)"
            fi
    
            AC_MSG_RESULT([no])            
            AC_MSG_ERROR([telluriccorr $telluriccorr_notfound was not found on your system.])
            
        else
            AC_MSG_RESULT([yes])            

            TELLURICCORR_INCLUDES="$telluriccorr_cflags"
            TELLURICCORR_CFLAGS="$telluriccorr_cflags"
            TELLURICCORR_LDFLAGS="$telluriccorr_ldflags"
            LIBTELLURICCORR="$telluriccorr_libs"
        fi
    
    else

        AC_MSG_WARN([telluriccorr checks have been disabled! This package may not build!])

        TELLURICCORR_INCLUDES=""
        TELLURICCORR_CFLAGS=""
        TELLURICCORR_LDFLAGS=""
        LIBTELLURICCORR=""

    fi

    AC_SUBST(TELLURICCORR_INCLUDES)
    AC_SUBST(TELLURICCORR_CFLAGS)
    AC_SUBST(TELLURICCORR_LDFLAGS)
    AC_SUBST(LIBTELLURICCORR)

])
# vim: et:ts=4:sw=4
