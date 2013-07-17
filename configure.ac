# CTX Fryer (the generator) configuration

AC_PREREQ([2.68])
AC_INIT([ctx-fryer], [0.1.0], [ctx-fryer@lopucha.net])
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/CXX/container/table.hpp])  # TODO: use generator .cpp when created
AC_CONFIG_HEADERS([src/CXX/config.h])
AC_PREFIX_DEFAULT([/usr/local])


# Resolve arguments

# Enable generator
AC_MSG_CHECKING([whether to build the generator])
AC_ARG_ENABLE([generator],
    AS_HELP_STRING([--enable-generator], [Build the generator (default: yes)]),
    [   # --enable-generator specified
        enable_generator=true
        case "${enableval}" in
            no|false|off)
                AC_MSG_RESULT([no])
                enable_generator=false
                ;;
            yes|true|on|"")
                AC_MSG_RESULT([yes])
                ;;
            *)
                AC_MSG_ERROR([unexpected --enable-generator argument: ${enableval}])
                ;;
        esac
    ],
    [   # --enable-generator not specified
        enable_generator=true
        AC_MSG_RESULT([yes])
    ])
AM_CONDITIONAL([ENABLE_GENERATOR], [test x$enable_generator = xtrue])

# Enable RT support
AC_MSG_CHECKING([whether to build real-time support])
AC_ARG_ENABLE([rt-support],
    AS_HELP_STRING([--enable-rt-support], [Create real-time parser support libs (default: yes)]),
    [   # --enable-rt-support specified
        enable_rt_support=true
        case "${enableval}" in
            no|false|off)
                AC_MSG_RESULT([no])
                enable_rt_support=false
                ;;
            yes|true|on|"")
                AC_MSG_RESULT([yes])
                ;;
            *)
                AC_MSG_ERROR([unexpected --enable-rt-support argument: ${enableval}])
                ;;
        esac
    ],
    [   # --enable-rt-support not specified
        enable_rt_support=true
        AC_MSG_RESULT([yes])
    ])
AM_CONDITIONAL([ENABLE_RT_SUPPORT], [test x$enable_rt_support = xtrue])

# Enable C target language RT support
AC_MSG_CHECKING([whether to build C language real-time support])
AC_ARG_ENABLE([C-rt-support],
    AS_HELP_STRING([--enable-C-rt-support], [Create real-time parser support C libs (default: yes)]),
    [   # --enable-C-rt-support specified
        enable_C_rt_support=true
        case "${enableval}" in
            no|false|off)
                AC_MSG_RESULT([no])
                enable_C_rt_support=false
                ;;
            yes|true|on|"")
                AC_MSG_RESULT([yes])
                ;;
            *)
                AC_MSG_ERROR([unexpected --enable-C-rt-support argument: ${enableval}])
                ;;
        esac
    ],
    [   # --enable-C-rt-support not specified
        enable_C_rt_support=true
        AC_MSG_RESULT([yes])
    ])
AM_CONDITIONAL([ENABLE_C_RT_SUPPORT], [test x$enable_C_rt_support = xtrue])

# Enable debug-only code
AC_MSG_CHECKING([whether to enable debug code])
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Enable compilation of debugging code (default: no)]),
    [   # --enable-debug specified
        enable_debug=false
        case "${enableval}" in
            no|false|off)
                AC_MSG_RESULT([no])
                ;;
            yes|true|on|"")
                AC_MSG_RESULT([yes])
                CXXFLAGS="${CXXFLAGS} -DDEBUG"
                enable_debug=true
                ;;
            *)
                AC_MSG_ERROR([unexpected --enable-debug argument: ${enableval}])
                ;;
        esac
    ],
    [   # --enable-debug not specified
        enable_debug=false
        AC_MSG_RESULT([no])
    ])
AM_CONDITIONAL([DEBUG], [test x$enable_debug = xtrue])

# Set optimisation level
AC_MSG_CHECKING([for optimisation level])
AC_ARG_WITH([optim-level],
    AS_HELP_STRING([--with-optim-level], [Set compiler optimisations level (default: 2)]),
    [   # --with-optim-level specified
        case "${withval}" in
            ""|0|1|2|3|s|fast)
                AC_MSG_RESULT([$withval])
                CFLAGS="${CXXFLAGS} -O${withval}"
                ;;
            *)
                AC_MSG_ERROR([unexpected --with-optim-level argument: ${withval}])
                CFLAGS="${CXXFLAGS} -O2"
                ;;
        esac
    ],
    [   # --with-optim-level not specified
        AC_MSG_RESULT([2])
        CFLAGS="${CXXFLAGS} -O2"
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
                CFLAGS="${CXXFLAGS} -s"
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
AC_CHECK_PROG([PERL], [perl], [/usr/bin/perl])
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
#AC_CHECK_LIB([lname], [lfunc])

# Checks for header files.
AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([])


# Run configure in run-time support directories
if test x$enable_rt_support = xtrue; then
    # C language run-time support libraries
    if test x$enable_C_rt_support = xtrue; then
        AC_MSG_NOTICE([running configure in C language run-time support directory])
        ( cd rt/C; ./configure --prefix "${prefix}" ) || \
            AC_MSG_ERROR([failed to configure in C language run-time support directory])
        AC_MSG_NOTICE([configure in C language run-time support directory done])
    fi
fi


# Create files

# Create the generator prototype Makefile
if test x$enable_generator = xtrue; then
    AC_MSG_NOTICE([creating the generator prototype Makefile using Perl MakeMaker])
    ( cd src/Perl; perl Makefile.PL PREFIX="${prefix}" ) || \
        AC_MSG_ERROR([failed to create the generator prototype Makefile])
    AC_MSG_NOTICE([generator prototype Makefile created])
fi

AC_CONFIG_FILES([
    Makefile
    examples/Makefile
    src/Makefile
    src/CXX/Makefile
    src/CXX/meta/Makefile
    src/CXX/container/Makefile
    src/CXX/unit_test/Makefile
    src/CXX/unit_test/container/Makefile
    src/Shell/Makefile
    src/Shell/ctx-fryer-paths.sh
    src/XML/Makefile
    rt/Makefile
    tlang/Makefile
    tlang/C/Makefile.xslt
])
AC_OUTPUT
