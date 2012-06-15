HOST_CC=gcc
CC=$(NACL_TOOLCHAIN_ROOT)/bin/$(ARCH)-gcc
CXX=$(NACL_TOOLCHAIN_ROOT)/bin/$(ARCH)-g++
LD=$(NACL_TOOLCHAIN_ROOT)/bin/$(ARCH)-g++
SDL_CONFIG=$(NACL_TOOLCHAIN_ROOT)/$(ARCH)/usr/bin/sdl-config
SDL_CFLAGS=`$(SDL_CONFIG) --cflags`
SDL_LIBS=-lSDL_mixer -lmikmod -lvorbisfile -lvorbis -logg `$(SDL_CONFIG) --libs` -lnosys
CFLAGS += -Wall -g -O0 -ffast-math -funroll-loops -Dstricmp=strcasecmp \
	-Dstrnicmp=strncasecmp -DUSE_SDL -I. $(SDL_CFLAGS) #-DUSE_NET
LIBS = -lm $(SDL_LIBS) \
-lppapi \
-lppapi_cpp

LDFLAGS += $(LIBS)


MODIFY_TARGET = gobpack jnbpack jnbunpack

CFILES = fireworks.c main.c menu.c filter.c resources.c sdl/gfx.c sdl/interrpt.c \
sdl/sound.c sdl/input.c extradefs.c
CCFILES=plugin.cc
OBJS = $(CFILES:%.c=%.o) $(CCFILES:%.cc=%.o)

TARGET = jumpnbump.nexe
BINARIES = $(TARGET) jumpnbump.svgalib jumpnbump.fbcon $(MODIFY_TARGET) \
	jnbmenu.tcl
PREFIX ?= /usr/local


%.o: %.c globals.h
	$(CC) $(CFLAGS) $(INCLUDES) $(DEBUG_FLAGS) -c -o $@ $<

%.o: %.cc globals.h
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEBUG_FLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@



all: $(TARGET)

www/$(TARGET): $(TARGET)
	cp $(TARGET) www/$(TARGET)

$(MODIFY_TARGET): globals.h
	cd modify && make


globals.h: globals.pre
	sed -e "s#%%PREFIX%%#$(PREFIX)#g" < globals.pre > globals.h

jnbmenu.tcl: jnbmenu.pre
	sed -e "s#%%PREFIX%%#$(PREFIX)#g" < jnbmenu.pre > jnbmenu.tcl

data/jumpbump.dat: jnbpack
	cd data && make

clean:
	cd modify && make clean
	cd data && make clean
	rm -f $(TARGET) $(OBJS) globals.h jnbmenu.tcl

doc:
	rman jumpnbump.6 -f HTML >jumpnbump.html

tools/genres: tools/genres.c
	$(HOST_CC) -o tools/genres tools/genres.c

resources.c: data/jumpbump.dat tools/genres
	tools/genres data/jumpbump.dat resources.c
