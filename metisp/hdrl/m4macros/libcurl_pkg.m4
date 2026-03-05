# ESO_CHECK_LIBCURL
#------------------
# Checks for the libcurl library and header files.
AC_DEFUN([ESO_CHECK_LIBCURL],
[

    AC_ARG_WITH(libcurl,
                AS_HELP_STRING([--with-libcurl],
                               [location where libcurl is installed]),
                [
                    libcurl_dir=$withval
                ])

    export PKG_CONFIG_PATH="$libcurl_dir/lib/pkgconfig:$libcurl_dir/lib64/pkgconfig:$LIBCURLDIR/lib/pkgconfig/:$LIBCURLDIR/lib64/pkgconfig/:$PKG_CONFIG_PATH"

    AC_SUBST(PKG_CONFIG_PATH)
    PKG_CHECK_MODULES([LIBCURL], [libcurl], 
                      AC_DEFINE([HAVE_LIBLIBCURL], [1], [1 if LIBCURL present]),
                      AC_MSG_ERROR([No LIBCURL available]))

])
