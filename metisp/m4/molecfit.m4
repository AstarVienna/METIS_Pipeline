# MOLECFIT_CHECK_LIBS(version)
#---------------------------------------
# Checks for the MOLECFIT library and header files.
AC_DEFUN([MOLECFIT_CHECK_LIBS],
[

    molecfit_check_version="$1"
    molecfit_check_header="molecfit.h"
    molecfit_check_libraries="-lmolecfit"

    molecfit_pkgconfig="molecfit"
    
    
    AC_REQUIRE([ESO_PROG_PKGCONFIG])
    AC_REQUIRE([CPL_CHECK_LIBS])

    AC_ARG_WITH(molecfit,
                AS_HELP_STRING([--with-molecfit],
                               [location where molecfit is installed]),
                [
                    molecfit_with_molecfit=$withval
                ])

    AC_ARG_WITH(molecfit-includes,
                AS_HELP_STRING([--with-molecfit-includes],
                               [location of the molecfit header files]),
                molecfit_with_molecfit_includes=$withval)

    AC_ARG_WITH(molecfit-libs,
                AS_HELP_STRING([--with-molecfit-libs],
                               [location of the molecfit library]),
                molecfit_with_molecfit_libs=$withval)

    AC_ARG_ENABLE(molecfit-test,
                  AS_HELP_STRING([--disable-molecfit-test],
                                 [disables checks for the molecfit library and headers]),
                  molecfit_enable_molecfit_test=$enableval,
                  molecfit_enable_molecfit_test=yes)

    AC_ARG_VAR([MOLECFITDIR], [Location where molecfit is installed])

    if test "x$molecfit_enable_molecfit_test" = xyes; then

        AC_MSG_CHECKING([for molecfit])

        # If include directories and libraries are given as arguments, use them
        # initially. Otherwise assume a standard system installation of the
        # package. This may then updated in the following.
        
        molecfit_libs="$molecfit_check_libraries"
        molecfit_cflags="-I/usr/include/molecfit -I/usr/include"
        molecfit_ldflags=""
    
        if test -n "${PKGCONFIG}"; then
    
            $PKGCONFIG --exists $molecfit_pkgconfig
    
            if test x$? = x0; then
                molecfit_cflags="`$PKGCONFIG --cflags $molecfit_pkgconfig`"
                molecfit_ldflags="`$PKGCONFIG --libs-only-L $molecfit_pkgconfig`"
                molecfit_libs="`$PKGCONFIG --libs-only-l $molecfit_pkgconfig`"
            fi
        
        fi

        # Directories given as arguments replace a standard system installation
        # setup if they are given.
                
        if test -n "$MOLECFITDIR"; then
            molecfit_cflags="-I$MOLECFITDIR/include/molecfit -I$MOLECFITDIR/include"
            molecfit_ldflags="-L$MOLECFITDIR/lib64 -L$MOLECFITDIR/lib"
        fi
        
        if test -n "$molecfit_with_molecfit"; then    
            molecfit_cflags="-I$molecfit_with_molecfit/include/molecfit -I$molecfit_with_molecfit/include"
            molecfit_ldflags="-L$molecfit_with_molecfit/lib64 -L$molecfit_with_molecfit/lib"
        fi    
        
        if test -n "$molecfit_with_molecfit_includes"; then
            molecfit_cflags="-I$molecfit_with_molecfit_includes"
        fi
        
        if test -n "$molecfit_with_molecfit_libs"; then
            molecfit_ldflags="-L$molecfit_with_molecfit_libs"
        fi
        
        
        # Check whether the header files and the library are present and
        # whether they can be used.
        
        molecfit_have_molecfit_libraries="no"
        molecfit_have_molecfit_headers="no"
            
        AC_LANG_PUSH(C)
        
        molecfit_cflags_save="$CFLAGS"
        molecfit_ldflags_save="$LDFLAGS"
        molecfit_libs_save="$LIBS"
    
        CFLAGS="$CPL_INCLUDES $molecfit_cflags"
        LDFLAGS="$molecfit_ldflags $CPL_LDFLAGS"
        LIBS="$molecfit_libs $LIBCPLUI $LIBCPLCORE"
    
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
                          [[
                          #include <$molecfit_check_header>
                          ]],
                          [
                           char c;
                          ])],
                          [
                           molecfit_have_molecfit_headers="yes"
                          ],
                          [
                           molecfit_have_molecfit_headers="no"
                          ])
    
        AC_LINK_IFELSE([AC_LANG_PROGRAM(
                       [[
                       ]],
                       [
                        mf_cleanup((void *)0);
                       ])],
                       [
                        molecfit_have_molecfit_libraries="yes"
                       ],
                       [
                        molecfit_have_molecfit_libraries="no"
                       ])
                        
        AC_LANG_POP(C)
       
        CFLAGS="$molecfit_cflags_save"
        LDFLAGS="$molecfit_ldflags_save"
        LIBS="$molecfit_libs_save"
        
        if test x"$molecfit_have_molecfit_libraries" = xno || \
            test x"$molecfit_have_molecfit_headers" = xno; then
            molecfit_notfound=""
    
            if test x"$molecfit_have_molecfit_headers" = xno; then
                if test x"$molecfit_have_molecfit_libraries" = xno; then
                    molecfit_notfound="(headers and libraries)"
                else
                    molecfit_notfound="(headers)"
                fi
            else
                molecfit_notfound="(libraries)"
            fi
    
            AC_MSG_RESULT([no])            
            AC_MSG_ERROR([molecfit $molecfit_notfound was not found on your system.])
            
        else
            AC_MSG_RESULT([yes])            

            MOLECFIT_INCLUDES="$molecfit_cflags"
            MOLECFIT_CFLAGS="$molecfit_cflags"
            MOLECFIT_LDFLAGS="$molecfit_ldflags"
            LIBMOLECFIT="$molecfit_libs"
        fi
    
    else

        AC_MSG_WARN([molecfit checks have been disabled! This package may not build!])

        MOLECFIT_INCLUDES=""
        MOLECFIT_CFLAGS=""
        MOLECFIT_LDFLAGS=""
        LIBMOLECFIT=""

    fi

    AC_SUBST(MOLECFIT_INCLUDES)
    AC_SUBST(MOLECFIT_CFLAGS)
    AC_SUBST(MOLECFIT_LDFLAGS)
    AC_SUBST(LIBMOLECFIT)

])
# vim: et:ts=4:sw=4
