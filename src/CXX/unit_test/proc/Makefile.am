# Compiler & linker flags
AM_CXXFLAGS = -g -Wall -Werror
AM_LDFLAGS  = -lrt

# Unit test scripts
TESTS = \
    daemon.sh \
    log.sh


# Unit test programs
check_PROGRAMS = \
    daemon \
    log

daemon_SOURCES = \
    daemon.cxx

log_SOURCES = \
    log.cxx

LDADD = ../../libctx-fryer.la
