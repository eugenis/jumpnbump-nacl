
LANGS = $(subst .po,,$(wildcard *.po))
MOS   = $(subst .po,.mo,$(wildcard *.po))

all: $(MOS)

clean:
	rm -f $(MOS)

install:
	for lang in $(LANGS) ; do \
	  install -D -o root -g root -m 0644 $$lang.mo $(PREFIX)/share/locale/$$lang/LC_MESSAGES/jumpnbump-menu.mo ; \
	done

uninstall:
	for lang in $(LANGS) ; do \
	  rm $(PREFIX)/share/locale/$$lang/LC_MESSAGES/jumpnbumo-menu.mo ; \
	done

%.mo: %.po
	msgfmt -o $@ $<

