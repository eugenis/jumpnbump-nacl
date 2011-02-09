NACL_CC=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl64-gcc
NACL_CPP=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl64-g++
SDL_CFLAGS = -I$(SDL_PREFIX)/include/SDL
SDL_LIBS = $(SDL_PREFIX)/lib/libSDL_mixer.a $(SDL_PREFIX)/lib/libSDL.a -lpthread
CFLAGS = -Wall -g -O0 -ffast-math -funroll-loops -Dstricmp=strcasecmp \
	-Dstrnicmp=strncasecmp -DUSE_SDL -I. $(SDL_CFLAGS) #-DUSE_NET
LIBS = -lm $(SDL_LIBS) \
-lppruntime \
-lppapi_cpp \
-lgoogle_nacl_platform \
-lgio \
-lpthread \
-lsrpc


 #-lSDL_mixer -lSDL_net
SDL_TARGET = sdl.a
MODIFY_TARGET = gobpack jnbpack jnbunpack
OBJS = fireworks.o main.o menu.o filter.o resources.o
TARGET = jumpnbump_x86_64.nexe
BINARIES = $(TARGET) jumpnbump.svgalib jumpnbump.fbcon $(MODIFY_TARGET) \
	jnbmenu.tcl
PREFIX ?= /usr/local

PLUGIN_OBJS = plugin/pi_generator_x86_64.o \
          plugin/pi_generator_module_x86_64.o


.PHONY: data

all: $(TARGET)

install: www/$(TARGET)

www/$(TARGET): $(TARGET)
	cp $(TARGET) www/$(TARGET)

$(SDL_TARGET): globals.h
	cd sdl && make

$(MODIFY_TARGET): globals.h
	cd modify && make

$(PLUGIN_OBJS): plugin/*.cc plugin/*.h
	make -C plugin

$(TARGET): $(OBJS) $(PLUGIN_OBJS) $(SDL_TARGET) data globals.h
	$(NACL_CPP) -o $(TARGET) $(OBJS) $(PLUGIN_OBJS) $(SDL_TARGET) $(LIBS)

$(OBJS): globals.h

%.o: %.c
	$(NACL_CC) $(CFLAGS) -m64 $(INCLUDES) -c -o $@ $<

globals.h: globals.pre
	sed -e "s#%%PREFIX%%#$(PREFIX)#g" < globals.pre > globals.h

jnbmenu.tcl: jnbmenu.pre
	sed -e "s#%%PREFIX%%#$(PREFIX)#g" < jnbmenu.pre > jnbmenu.tcl

data: jnbpack
	cd data && make

clean:
	cd sdl && make clean
	cd modify && make clean
	cd data && make clean
	cd plugin && make clean
	rm -f $(TARGET) *.o globals.h jnbmenu.tcl

# install:
# 	mkdir -p $(PREFIX)/games/
# 	mkdir -p $(PREFIX)/share/jumpnbump/
# 	mkdir -p $(PREFIX)/share/man/man6/
# 	install -o root -g games -m 755 $(BINARIES) $(PREFIX)/games/
# 	install -o root -g games -m 644 data/jumpbump.dat \
# 	$(PREFIX)/share/jumpnbump/jumpbump.dat
# 	install -o root -g root -m 644 jumpnbump.6 $(PREFIX)/share/man/man6/

# uninstall:
# 	cd $(PREFIX)/games && rm -f $(BINARIES)
# 	rm -rf $(PREFIX)/share/jumpnbump
# 	rm -f $(PREFIX)/share/man/man6/jumpnbump.6

doc:
	rman jumpnbump.6 -f HTML >jumpnbump.html

tools/genres: tools/genres.c
	$(CC) -o tools/genres tools/genres.c

resources.c: data/jumpbump.dat tools/genres
	tools/genres data/jumpbump.dat resources.c
