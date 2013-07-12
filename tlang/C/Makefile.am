AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS  = -I m4

# Compiler & linker flags
AM_CFLAGS  = -Wall -Werror
AM_LDFLAGS =

# Debug mode specific settings
# Note that CFLAGS already contain -DDEBUG in this mode
if DEBUG
endif


# Parser
lib_LTLIBRARIES = libparser.la

libparser_la_SOURCES = \
    lexical_items.h \
    fsa_table.h \
    fsa_table.c \
    nonterminals.h \
    extern_attr.h \
    parser_tables.h \
    parser_tables.c \
    parser.h \
    parser.c

include Makefile.xslt
