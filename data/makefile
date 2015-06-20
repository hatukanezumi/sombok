#-*- Makefile -*-

UNICODE_VERSIONS = 5.1.0 5.2.0 6.0.0 6.1.0 6.2.0 6.3.0 7.0.0 8.0.0
CURRENT_UNICODE_VERSION = 8.0.0

UNICODE_VERSION ?= $(CURRENT_UNICODE_VERSION)

WGET = wget
UCD_BASE = ftp://www.unicode.org/Public/`echo $(UNICODE_VERSION)|sed -e 's/[^.0-9].*//g'`/ucd/
UNICODEDATA = UnicodeData-$(UNICODE_VERSION).txt
LINEBREAK = LineBreak-$(UNICODE_VERSION).txt
EASTASIANWIDTH = EastAsianWidth-$(UNICODE_VERSION).txt
GRAPHEMEBREAK = GraphemeBreakProperty-$(UNICODE_VERSION).txt
GRAPHEMEBREAK_CUSTOM = GraphemeBreakProperty-$(UNICODE_VERSION).custom
RULES = Rules-$(UNICODE_VERSION).txt
EASTASIANWIDTH_CUSTOM = EastAsianWidth-$(UNICODE_VERSION).custom
LINEBREAK_CUSTOM = LineBreak-$(UNICODE_VERSION).custom
SCRIPTS = Scripts-$(UNICODE_VERSION).txt
#SCRIPTS_CUSTOM = Scripts-$(UNICODE_VERSION).custom
SCRIPTS_CUSTOM =
DATA_C = ../lib/$(UNICODE_VERSION).c
CONSTANTS_H = ../include/sombok_constants.h
CONFIGURE_IN = ../configure.ac
CONFIG_UNICODE = ../UNICODE

all: $(CONSTANTS_H) $(CONFIG_UNICODE)
	if [ -e UnicodeData-$(UNICODE_VERSION).txt ]; then \
	    $(MAKE) data || exit 1; \
	fi

data: $(DATA_C)

$(DATA_C): $(RULES) $(EASTASIANWIDTH) $(LINEBREAK) $(SCRIPTS) $(EASTASIANWIDTH_CUSTOM) $(LINEBREAK_CUSTOM) $(GRAPHEMEBREAK_CUSTOM) data.pl LBCLASSES
	perl data.pl lb,ea,gb,sc $(UNICODE_VERSION)

$(CONSTANTS_H) LBCLASSES: makefile constants.pl
	perl constants.pl lb,ea,gb,sc $(UNICODE_VERSIONS)

$(CONFIG_UNICODE): makefile
	echo "$(CURRENT_UNICODE_VERSION)" > $(CONFIG_UNICODE)
	touch $(CONFIGURE_IN)

custom-data: $(UNICODEDATA) $(LINEBREAK) $(EASTASIANWIDTH) $(GRAPHEMEBREAK) $(SCRIPTS) custom.pl
	for f in $(EASTASIANWIDTH_CUSTOM) $(LINEBREAK_CUSTOM) $(GRAPHEMEBREAK_CUSTOM) $(SCRIPTS_CUSTOM); do \
	    if [ -e $$f ]; then mv $$f $$f.old; fi; done
	perl custom.pl ea $(UNICODE_VERSION) > $(EASTASIANWIDTH_CUSTOM)
	perl custom.pl lb $(UNICODE_VERSION) > $(LINEBREAK_CUSTOM)
	perl custom.pl gb $(UNICODE_VERSION) > $(GRAPHEMEBREAK_CUSTOM)

custom:
	for v in $(UNICODE_VERSIONS); do \
		$(MAKE) custom-data UNICODE_VERSION=$$v; \
	done

unicodedata-data:
	$(WGET) -O $(UNICODEDATA) $(UCD_BASE)UnicodeData\*.txt
	$(WGET) -O $(LINEBREAK) $(UCD_BASE)LineBreak\*.txt
	$(WGET) -O $(EASTASIANWIDTH) $(UCD_BASE)EastAsianWidth\*.txt
	$(WGET) -O $(GRAPHEMEBREAK) $(UCD_BASE)auxiliary/GraphemeBreakProperty\*.txt
	$(WGET) -O $(SCRIPTS) $(UCD_BASE)Scripts\*.txt

unicodedata:
	for v in $(UNICODE_VERSIONS); do \
		$(MAKE) unicodedata-data UNICODE_VERSION=$$v; \
	done

clean-data:
	rm -f $(DATA_C)

clean:
	rm -f $(CONSTANTS_H)
	for v in $(UNICODE_VERSIONS); do \
		$(MAKE) clean-data UNICODE_VERSION=$$v; \
	done

