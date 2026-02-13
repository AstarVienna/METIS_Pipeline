# CPL_CHECK_CEXT(incdirs=[], libdirs=[])
#---------------------------------------
# Checks for the C extension library and header files.
AC_DEFUN([CPL_CHECK_CEXT],
[

    AC_MSG_CHECKING([for libcext])

    cpl_cext_check_header="cxtypes.h"
    cpl_cext_check_lib="libcext.a"

    cpl_cext_incdirs=""
    cpl_cext_libdirs=""

    cpl_cext_includes=""
    cpl_cext_libraries=""


	# Initialize directory search paths with the arguments provided
	
	if test -n "$1"; then
		cpl_cext_incdirs="$1"
	fi

	if test -n "$2"; then
		cpl_cext_libdirs="$2"
	fi


    AC_ARG_WITH(cext,
                AS_HELP_STRING([--with-cext],
                               [location where libcext is installed]),
                [
                    cpl_with_cext=$withval
                ])

    AC_ARG_WITH(cext-includes,
                AS_HELP_STRING([--with-cext-includes],
                               [location of the libcext header files]),
                cpl_with_cext_includes=$withval)

    AC_ARG_WITH(cext-libs,
                AS_HELP_STRING([--with-cext-libs],
                               [location of the libcext library]),
                cpl_with_cext_libs=$withval)

    AC_ARG_ENABLE(cext-test,
                  AS_HELP_STRING([--disable-cext-test],
                                 [disables checks for the libcext library and headers]),
                  cpl_enable_cext_test=$enableval,
                  cpl_enable_cext_test=yes)


    if test "x$cpl_enable_cext_test" = xyes; then
    
        # Check for the libcext includes

        if test -z "$cpl_with_cext_includes"; then
        
            if test -z "$cpl_with_cext"; then
            
            	if test -z "$cpl_cext_incdirs"; then
            
	                # Try some known system locations
                
    	            cpl_cext_incdirs="/opt/cext/include"
        	        cpl_cext_incdirs="$cpl_cext_incdirs /usr/local/include/cext"
            	    cpl_cext_incdirs="$cpl_cext_incdirs /usr/local/include"
                	cpl_cext_incdirs="$cpl_cext_incdirs /usr/include/cext"
                	cpl_cext_incdirs="$cpl_cext_incdirs /usr/include"

                	test -n "$CPLDIR" && \
                    	cpl_cext_incdirs="$CPLDIR/include/cext \
                        	              $CPLDIR/include \
                            	          $cpl_cext_incdirs"

				fi
				
            else

                cpl_cext_incdirs="$cpl_with_cext/include/cext"
                cpl_cext_incdirs="$cpl_cext_incdirs $cpl_with_cext/include"

            fi
            
        else
            cpl_cext_incdirs="$cpl_with_cext_includes"
        fi

        ESO_FIND_FILE($cpl_cext_check_header, $cpl_cext_incdirs,
                      cpl_cext_includes)


        # Check for the libcext library

        if test -z "$cpl_with_cext_libs"; then

            if test -z "$cpl_with_cext"; then
            
            	if test -z "$cpl_cext_libdirs"; then

	                # Try some known system locations

    	            cpl_cext_libdirs="/opt/cext/lib"
                	cpl_cext_libdirs="$cpl_cext_libdirs /usr/local/lib64"
                	cpl_cext_libdirs="$cpl_cext_libdirs /usr/local/lib"
                	cpl_cext_libdirs="$cpl_cext_libdirs /usr/local/lib32"
                	cpl_cext_libdirs="$cpl_cext_libdirs /usr/lib64"
                	cpl_cext_libdirs="$cpl_cext_libdirs /usr/lib"
                	cpl_cext_libdirs="$cpl_cext_libdirs /usr/lib32"

                	test -n "$CPLDIR" && \
                    	cpl_cext_libdirs="$CPLDIR/lib64 \
                    					  $CPLDIR/lib \
                    					  $CPLDIR/lib32 \
                                          $cpl_cext_libdirs"
                                          
				fi
				
            else

                cpl_cext_libdirs="$cpl_with_cext/lib64"
                cpl_cext_libdirs="$cpl_cext_libdirs $cpl_with_cext/lib"
                cpl_cext_libdirs="$cpl_cext_libdirs $cpl_with_cext/lib32"

            fi
            
        else
            cpl_cext_libdirs="$cpl_with_cext_libs"
        fi

        ESO_FIND_FILE($cpl_cext_check_lib, $cpl_cext_libdirs,
                      cpl_cext_libraries)


        if test x"$cpl_cext_includes" = xno || \
            test x"$cpl_cext_libraries" = xno; then
            cpl_cext_notfound=""

            if test x"$cpl_cext_includes" = xno; then
                if test x"$cpl_cext_libraries" = xno; then
                    cpl_cext_notfound="(headers and libraries)"
                else
                    cpl_cext_notfound="(headers)"
                fi
            else
                cpl_cext_notfound="(libraries)"
            fi

            AC_MSG_ERROR([libcext $cpl_cext_notfound was not found on your system. Please check!])
        else
            AC_MSG_RESULT([libraries $cpl_cext_libraries, headers $cpl_cext_includes])
        fi


        # Set up the symbols

        CX_INCLUDES="-I$cpl_cext_includes"
        CX_LDFLAGS="-L$cpl_cext_libraries"
        LIBCEXT="-lcext"
        
        
        AC_MSG_CHECKING([whether libcext can be used])
        AC_LANG_PUSH(C)
        
        cpl_cext_cflags_save="$CFLAGS"
        cpl_cext_ldflags_save="$LDFLAGS"
        cpl_cext_libs_save="$LIBS"

        CFLAGS="$CX_INCLUDES"
        LDFLAGS="$CX_LDFLAGS"
        LIBS="$LIBCEXT"
        
        AC_LINK_IFELSE([AC_LANG_PROGRAM(
                       [[
                       #include <cxutils.h>
                       ]],
                       [
                       cx_program_set_name("MyProgram");
                       ])],
                       [cpl_cext_is_usable="yes"],
                       [cpl_cext_is_usable="no"])

        AC_MSG_RESULT([$cpl_cext_is_usable])
        
        AC_LANG_POP(C)
        
        CFLAGS="$cpl_cext_cflags_save"
        LDFLAGS="$cpl_cext_ldflags_save"
        LIBS="$cpl_cext_libs_save"

        if test x"$cpl_cext_is_usable" = xno; then
            AC_MSG_ERROR([Linking with libcext failed! Please check architecture!])
        fi
        
    else
    
        AC_MSG_RESULT([disabled])
        AC_MSG_WARN([libcext checks have been disabled! This package may not build!])
        CX_INCLUDES=""
        CX_LDFLAGS=""
        LIBCEXT=""
        
    fi

    AC_SUBST(CX_INCLUDES)
    AC_SUBST(CX_LDFLAGS)
    AC_SUBST(LIBCEXT)

])


