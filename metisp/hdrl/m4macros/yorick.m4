# ===========================================================================
#     http://www.gnu.org/software/autoconf-archive/ax_normalize_path.html
# ===========================================================================#
# SYNOPSIS
#
#   AX_NORMALIZE_PATH(VARNAME, [REFERENCE_STRING])
#
# DESCRIPTION
#
#   Perform some cleanups on the value of $VARNAME (interpreted as a path):
AU_ALIAS([ADL_NORMALIZE_PATH], [AX_NORMALIZE_PATH])
AC_DEFUN([AX_NORMALIZE_PATH],
[case ":[$]$1:" in
# change empty paths to '.'
  ::) $1='.' ;;
# strip trailing slashes
  :*[[\\/]]:) $1=`echo "[$]$1" | sed 's,[[\\/]]*[$],,'` ;;
  :*:) ;;
esac
# squeze repeated slashes
case ifelse($2,,"[$]$1",$2) in
# if the path contains any backslashes, turn slashes into backslashes
 *\\*) $1=`echo "[$]$1" | sed 's,\(.\)[[\\/]][[\\/]]*,\1\\\\,g'` ;;
# if the path contains slashes, also turn backslashes into slashes
 *) $1=`echo "[$]$1" | sed 's,\(.\)[[\\/]][[\\/]]*,\1/,g'` ;;
esac])


# CHECK_YORICK
#------------------
# Checks for the yorick executable.
# If the first argument is "optional" the check is allowed to fail, otherwise
# missing yorick is a fatal error
# It sets:
# YORICK_BIN: The file name of the yorick binary executable
# YORICK_DIR: The directory where the yorick binary executable is
AC_DEFUN([CHECK_YORICK],
[

    AC_ARG_WITH(yorick,
                AS_HELP_STRING([--with-yorick],
                               [location where yorick is installed]),
                [
                    yorick_bin=$withval/bin
                    yorick_dir=$withval
                ])
    AC_ARG_WITH(yorick-bindir,
                AS_HELP_STRING([--with-yorick-bindir],
                               [location where yorick binary is installed]),
                [
                    yorick_bin=$withval
                ])

    if test "x$with_yorick" != xno; then

        AC_PATH_PROG(YORICK_BIN, yorick, [], [$yorick_bin:$PATH])
        if test -z "$YORICK_BIN"; then
            if test "x$1" = xoptional; then
                AC_MSG_NOTICE([optional yorick executable not found])
            else
                AC_MSG_ERROR([yorick executable not found])
            fi
        else
            AX_NORMALIZE_PATH(YORICK_BIN)

            if test -z "$yorick_dir"; then
                YORICK_DIR=$(AS_DIRNAME(["$YORICK_BIN"]))
                YORICK_DIR=$(AS_DIRNAME(["$YORICK_DIR"]))
            else
                YORICK_DIR="$yorick_dir"
            fi

            AC_SUBST(YORICK_DIR)
            AC_SUBST(YORICK_BIN)

            AC_DEFINE_UNQUOTED(YORICK_DIR, "$YORICK_DIR",
                               [Absolute path to the Yorick installation dir])

            AC_DEFINE_UNQUOTED(YORICK_BIN, "$YORICK_BIN",
                               [Absolute path to the Yorick executable])

        fi

    else
        AC_MSG_WARN([yorick checks have been disabled])
    fi
])

# CHECK_YORICK_HEADERS
#------------------
# Checks for the location of the Yorick C headers.
# This implicitly requires the Yorick binary to be searched for.
AC_DEFUN([CHECK_YORICK_HEADERS],
[
    AC_REQUIRE([CHECK_YORICK])

    AC_ARG_WITH(yorick-includes,
                AS_HELP_STRING([--with-yorick-includes],
                               [location where yorick header files are installed]),
                [
                    yorick_inc=$withval
                ])

    if test -z "$yorick_inc"; then
        yorick_inc=$(find "$YORICK_DIR/include" -name yapi.h)
        yorick_inc=$(AS_DIRNAME(["$yorick_inc"]))
        AC_CHECK_HEADER([$yorick_inc/yapi.h],
                        [YORICK_INCLUDES="-I$yorick_inc"],
                        AC_MSG_ERROR([yorick include path to header files not found]))
    else
        YORICK_INCLUDES="-I$yorick_inc"
    fi

    AC_SUBST(YORICK_INCLUDES)

])]

# CHECK_YORICK_PLUGIN
#------------------
# Checks for a yorick plugin
# The first argument is the file name of the yorick plugin
# If the second argument is "optional" the check is allowed to fail, otherwise
# missing yorick plugin is a fatal error
AC_DEFUN([CHECK_YORICK_PLUGIN],
[
    AC_REQUIRE([CHECK_YORICK])

    if test "x$with_yorick" != xno; then

        AC_MSG_CHECKING([$1 plugin for yorick $YORICK_BIN])

        AC_LANG_PUSH(C)
    
        AC_RUN_IFELSE([AC_LANG_PROGRAM(
            [[
            #include <unistd.h>
            #include <sys/types.h>
            #include <stdio.h>
            #include <sys/wait.h>
            #include <stdlib.h>
            ]],
            [
            int status;
            pid_t pid;
            char *const program = YORICK_BIN;
            char *param[[]] = {YORICK_BIN, "-batch", $1, (char*)0};
            switch(pid=fork())
            {
                case 0: /* Child process (Yorick) */
                    if (execv(program,param) == -1 ) /* execv itself failed */
                        exit(1);
                    return 0;
                case -1: /* Fork failed */
                    exit(2);
                default:
                    wait(&status);
                    if(status) /* Error in child process */
                        exit(3);
            }
            return 0;
            ])],
            [AC_MSG_RESULT([available])],
            [
                if test "x$2" = xoptional; then
                    AC_MSG_NOTICE([Optional $1 plugin for yorick not installed])
                else
                    AC_MSG_FAILURE([$1 plugin for yorick not installed])
                fi
            ])

        AC_LANG_POP(C)

    fi
])

# CHECK_YORICK_YETI
#------------------
# Checks for the yorick yeti plugin
AC_DEFUN([CHECK_YORICK_YETI],
[
    CHECK_YORICK_PLUGIN(["yeti.i"], $1)
])

#
# CHECK_YORICK_OPTIMPACK
#------------------
# Checks for the OptimPack plugin
AC_DEFUN([CHECK_YORICK_OPTIMPACK],
[
    CHECK_YORICK_PLUGIN(["OptimPack1.i"], $1)
])
