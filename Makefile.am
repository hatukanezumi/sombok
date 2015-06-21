lib_LTLIBRARIES = libsombok.la
libsombok_la_SOURCES = lib/break.c lib/charprop.c lib/gcstring.c \
	lib/linebreak.c lib/utf8.c lib/utils.c \
	lib/southeastasian.c lib/@UNICODE_VERSION@.c
if MALLOC_DEBUG
libsombok_la_SOURCES += src/mymalloc.c
endif
libsombok_la_CFLAGS = -I. -Iinclude @LIBTHAI_CFLAGS@
libsombok_la_LDFLAGS = -version-info 4:7:1 @LIBTHAI_LIBS@

include_HEADERS = include/sombok_constants.h
nodist_include_HEADERS = include/sombok.h
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

if ENABLE_DOC
html_DATA = doc/html/*
MOSTLYCLEANFILES = stamp-html $(html_DATA)

$(html_DATA): stamp-html

stamp-html: doc/doxyfile $(include_HEADERS) $(nodist_include_HEADERS) \
	    $(libsombok_la_SOURCES)
	$(DOXYGEN) doc/doxyfile
	touch stamp-html	
endif

EXTRA_DIST = sombok.spec debian/* \
	lib/?.?.?.c \
	tests/lb.sh $(TESTS) test-data/* \
	data/makefile data/constants.pl data/custom.pl data/data.pl \
	data/*.custom data/Rules-*.txt data/LBCLASSES \
	ARTISTIC ChangeLog.REL1 INSTALL.ja_JP README.ja_JP \
	UNICODE VERSION

