# CTX Fryer C language project configuration

AC_PREREQ([2.68])
AC_INIT([libparser], [0.1.0], [ctx-fryer@lopucha.net])
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([parser.c])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr/local])


# Resolve arguments

# Set additional include paths
AC_MSG_CHECKING([for additional include paths])
AC_ARG_WITH([include],
    AS_HELP_STRING([--with-include], [Set additional include paths (':' separated list)]),
    [   # --with-include specified
        includes=`echo "${withval}" | sed -e 's/^/-I/' -e 's/:/ -I/g'`
        CFLAGS="${CFLAGS} ${includes}"
        AC_MSG_RESULT([${includes}])
    ],
    [   # --with-optim-level not specified
        AC_MSG_RESULT([none])
    ])

# Enable debug-only code
AC_MSG_CHECKING([whether to enable debug code])
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Enable compilation of debugging code (default: no)]),
    [   # --enable-debug specified
        debug=false
        case "${enableval}" in
            no|false|off)
                AC_MSG_RESULT([no])
                ;;
            yes|true|on|"")
                AC_MSG_RESULT([yes])
                CFLAGS="${CFLAGS} -DDEBUG"
                debug=true
                ;;
            *)
                AC_MSG_ERROR([unexpected --enable-debug argument: ${enableval}])
                ;;
        esac
    ],
    [   # --enable-debug not specified
        debug=false
        AC_MSG_RESULT([no])
    ])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

# Set optimisation level
AC_MSG_CHECKING([for optimisation level])
AC_ARG_WITH([optim-level],
    AS_HELP_STRING([--with-optim-level], [Set compiler optimisations level (default: 2)]),
    [   # --with-optim-level specified
        case "${withval}" in
            ""|0|1|2|3|s|fast)
                AC_MSG_RESULT([$withval])
                CFLAGS="${CFLAGS} -O${withval}"
                ;;
            *)
                AC_MSG_ERROR([unexpected --with-optim-level argument: ${withval}])
                CFLAGS="${CFLAGS} -O2"
                ;;
        esac
    ],
    [   # --with-optim-level not specified
        AC_MSG_RESULT([2])
        CFLAGS="${CFLAGS} -O2"
    ])

# Strip binaries
AC_MSG_CHECKING([whether to strip binaries])
AC_ARG_ENABLE([strip],
    AS_HELP_STRING([--enable-strip], [Strip debug info from binaries (default: no)]),
    [   # --enable-strip specified (with or without argument)
        case "${enableval}" in
            no|false|off)
                AC_MSG_RESULT([no])
                ;;
            yes|true|on|"")
                AC_MSG_RESULT([yes])
                CFLAGS="${CFLAGS} -s"
                ;;
            *)
                AC_MSG_ERROR([unexpected --enable-strip argument: ${enableval}])
                ;;
        esac
    ],
    [   # --enable-strip not specified
        AC_MSG_RESULT([no])
    ])


# Checks for programs.
AC_PROG_CC

AC_PATH_PROG([XSLTPROC], [xsltproc])
test -n "${XSLTPROC}" || AC_MSG_ERROR([xsltproc is required to generate target code])

# Checks for libraries.
#AC_CHECK_LIB([lname], [lfunc])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strerror])

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
