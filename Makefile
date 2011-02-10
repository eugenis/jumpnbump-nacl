HOST_CC=gcc
NACL_CC=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl-gcc
NACL_CXX=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl-g++
NACL_LD=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl-g++
SDL_CFLAGS = -I$(SDL_PREFIX)/include/SDL
SDL_LIBS = $(SDL_PREFIX)/lib/libSDL_mixer.a $(SDL_PREFIX)/lib/libSDL.a $(SDL_PREFIX)/lib/libmikmod.a -lpthread
CFLAGS += -Wall -g -O0 -ffast-math -funroll-loops -Dstricmp=strcasecmp \
	-Dstrnicmp=strncasecmp -DUSE_SDL -I. $(SDL_CFLAGS) #-DUSE_NET
LIBS = -lm $(SDL_LIBS) \
-lppruntime \
-lppapi_cpp \
-lgoogle_nacl_platform \
-lgio \
-lpthread \
-lsrpc \
-lnosys
LDFLAGS += $(LIBS)


MODIFY_TARGET = gobpack jnbpack jnbunpack

CFILES = fireworks.c main.c menu.c filter.c resources.c sdl/gfx.c sdl/interrpt.c \
sdl/sound.c sdl/input.c
CCFILES=plugin/pi_generator.cc plugin/pi_generator_module.cc
OBJS = $(CFILES:%.c=%.o) $(CCFILES:%.cc=%.o)

TARGET = jumpnbump.nexe
BINARIES = $(TARGET) jumpnbump.svgalib jumpnbump.fbcon $(MODIFY_TARGET) \
	jnbmenu.tcl
PREFIX ?= /usr/local


%.o: %.c globals.h
	$(NACL_CC) $(CFLAGS) $(INCLUDES) $(DEBUG_FLAGS) -c -o $@ $<

%.o: %.cc globals.h
	$(NACL_CXX) $(CFLAGS) $(INCLUDES) $(DEBUG_FLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(NACL_LD) $^ $(LDFLAGS) -o $@


.PHONY: data

all: $(TARGET)

www/$(TARGET): $(TARGET)
	cp $(TARGET) www/$(TARGET)

$(MODIFY_TARGET): globals.h
	cd modify && make


globals.h: globals.pre
	sed -e "s#%%PREFIX%%#$(PREFIX)#g" < globals.pre > globals.h

jnbmenu.tcl: jnbmenu.pre
	sed -e "s#%%PREFIX%%#$(PREFIX)#g" < jnbmenu.pre > jnbmenu.tcl

data: jnbpack
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
