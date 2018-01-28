ASCIIDOC := /usr/bin/asciidoc

SRCS := index.txt focus_js_features.txt focus_js_quick_start.txt \
        focus_js_command_line.txt focus_js_configuration.txt \
        focus_js_links.txt focus_js_license.txt focus_js_brochure.txt

MY_DOCS := $(SRCS:.txt=.html)

%.html: %.txt
	${ASCIIDOC} $<

.PHONY: clean

all: $(MY_DOCS)
	@echo done.

clean:
	rm -f ${MY_DOCS}
