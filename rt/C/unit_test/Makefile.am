# Compiler & linker flags
AM_CFLAGS  = -g -Wall -Werror
AM_LDFLAGS = -L../src -lctx-fryer-c-rt -lctx-fryer-c-rt-ut


# Unit test scripts
TESTS = test.sh


# Unit test programs
check_PROGRAMS = \
    test.srcfile \
    test.lexical_analyser \
    test.syntax_analyser

test_srcfile_SOURCES = \
    test.srcfile.c

test_lexical_analyser_SOURCES = \
    test.lexical_analyser.c \
    test.fsa_table.c

test_syntax_analyser_SOURCES = \
    test.syntax_analyser.c \
    test.fsa_table.c \
    test.parser_tables.c \
    test.extern_attr.c
