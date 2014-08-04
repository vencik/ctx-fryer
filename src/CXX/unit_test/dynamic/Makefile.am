# Compiler & linker flags
AM_CXXFLAGS = -g -Wall -Werror
AM_LDFLAGS  =

# Unit test scripts
TESTS = \
    pointer.sh \
    mempool.sh


# Unit test programs
check_PROGRAMS = \
    mempool \
    pointer

mempool_SOURCES = \
    mempool.cxx

pointer_SOURCES = \
    pointer.cxx

LDADD = ../../libctx-fryer.la
