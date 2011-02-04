lib_LTLIBRARIES = libsombok.la
libsombok_la_SOURCES = lib/break.c lib/charprop.c lib/gcstring.c \
	lib/linebreak.c lib/southeastasian.c lib/utils.c \
	lib/@UNICODE_VERSION@.c
if MALLOC_DEBUG
libsombok_la_SOURCES += src/mymalloc.c
endif
libsombok_la_CFLAGS = -I. -Iinclude @LIBTHAI_CFLAGS@
libsombok_la_LDFLAGS = -version-info 3:0:0 @LIBTHAI_LIBS@

include_HEADERS = include/sombok.h include/sombok_constants.h
nodist_include_HEADERS =
noinst_HEADERS = src/mymalloc.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = sombok.pc

check_PROGRAMS = sombok
sombok_SOURCES = src/sombok.c
if !HAVE_STRERROR
sombok_SOURCES += src/strerror.c
endif
if !HAVE_STRCASECMP
sombok_SOURCES += src/strcasecmp.c
endif
sombok_CFLAGS = -Iinclude
sombok_LDFLAGS =
sombok_LDADD = libsombok.la
TESTS = tests/*.t

#pkgdata_DATA =
#pkgdatasubdir =
#pkgdatasub_DATA =
EXTRA_DIST = sombok.spec \
	lib/5.1.0.c lib/5.2.0.c \
	tests/lb.sh $(TESTS) test-data/* \
	data/makefile data/constants.pl data/custom.pl data/data.pl \
	data/*.custom data/Rules-*.txt data/LBCLASSES \
	ChangeLog.REL1

