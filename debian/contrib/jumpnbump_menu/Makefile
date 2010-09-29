PREFIX ?= /usr

all:
	$(MAKE) -C po all

clean:
	$(MAKE) -C po clean

install:
	install -o root -g root -m 644 jumpnbump_menu.glade $(PREFIX)/share/games/jumpnbump/
	install -o root -g root -m 755 jumpnbump_menu.py $(PREFIX)/games/jumpnbump-menu

	$(MAKE) -C po install

uninstall:
	rm $(PREFIX)/share/games/jumpnbump/jumpnbump_menu.glade
	rm $(PREFIX)/games/jumpnbump-menu

	$(MAKE) -C po uninstall

