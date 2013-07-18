AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS  = -I m4


SUBDIRS = 

if ENABLE_GENERATOR
SUBDIRS += src tlang examples
endif

if ENABLE_RT_SUPPORT
SUBDIRS += rt
endif

if ENABLE_DOC
SUBDIRS += doc
endif
