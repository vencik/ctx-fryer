AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS  = -I m4

SUBDIRS = unit_test


# Compiler & linker flags
AM_CFLAGS  = -Wall -Werror
AM_LDFLAGS =

# Debug mode specific settings
# Note that CFLAGS already contain -DDEBUG in this mode
if DEBUG
endif


# Generated files
BUILT_SOURCES = \
    lexical_items.h \
    nonterminals.h \
    extern_attr.h \
    fsa_table.c \
    parser_tables.c

# Parser
lib_LTLIBRARIES = libparser.la

libparser_la_SOURCES = \
    fsa_table.c \
    parser_tables.c \
    parser.c

include Makefile.xslt
