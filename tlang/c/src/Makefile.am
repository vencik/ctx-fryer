# Compiler & linker flags
AM_CFLAGS  = -Wall -Werror
AM_LDFLAGS =

# Debug mode specific settings
# Note that CFLAGS already contain -DDEBUG in this mode
if DEBUG
endif


# Parser run-time support
lib_LTLIBRARIES = libctx-fryer-c-rt.la

libctx_fryer_c_rt_la_SOURCES = \
    objpool.c objpack.c buffer.c stack.c srcfile.c \
    ptree.c attribute.c builtin_attr.c \
    lexical_analyser.c token.c syntax_analyser.c


# Unit test support
check_LTLIBRARIES = libctx-fryer-c-rt-ut.la

libctx_fryer_c_rt_ut_la_SOURCES = \
    test.c
