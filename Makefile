HOST_CC=gcc
NACL_TOOLCHAIN_ROOT=$(NACL_SDK_ROOT)/toolchain/linux_x86_glibc
CC=$(NACL_TOOLCHAIN_ROOT)/bin/$(ARCH)-gcc
CXX=$(NACL_TOOLCHAIN_ROOT)/bin/$(ARCH)-g++
LD=$(NACL_TOOLCHAIN_ROOT)/bin/$(ARCH)-g++
SDL_CONFIG=$(NACL_TOOLCHAIN_ROOT)/$(ARCH)/usr/bin/sdl-config
SDL_CFLAGS=`$(SDL_CONFIG) --cflags`
SDL_LIBS=-lSDL_mixer -lmikmod -lvorbisfile -lvorbis -logg `$(SDL_CONFIG) --libs` -lSDL_net -lnacl-mounts
CFLAGS += -Wall -g -O0 -ffast-math -funroll-loops -Dstricmp=strcasecmp \
	-Dstrnicmp=strncasecmp -DUSE_SDL -I. $(SDL_CFLAGS) -DUSE_NET \
	-I $(NACL_TOOLCHAIN_ROOT)/$(ARCH)/usr/include/nacl-mounts
LIBS = -lm $(SDL_LIBS) \
-lppapi \
-lppapi_cpp

ifeq ($(ARCH), i686-nacl)
  EXTRA_GEN_NMF_ARGS=-L $(NACL_TOOLCHAIN_ROOT)/x86_64-nacl/lib32
else
  EXTRA_GEN_NMF_ARGS=
endif

LDFLAGS += $(LIBS)

OUT=out-$(ARCH)

MODIFY_TARGET = gobpack jnbpack jnbunpack

CFILES = fireworks.c main.c menu.c filter.c resources.c sdl/gfx.c sdl/interrpt.c \
sdl/sound.c sdl/input.c extradefs.c
CCFILES=plugin.cc
OBJS = $(addprefix $(OUT)/, $(CFILES:%.c=%.o) $(CCFILES:%.cc=%.o))

TARGET = jumpnbump-$(ARCH).nexe
BINARIES = $(TARGET) jumpnbump.svgalib jumpnbump.fbcon $(MODIFY_TARGET) \
	jnbmenu.tcl
PREFIX ?= /usr/local
STAGING = staging

$(OUT)/%.o: %.c globals.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEBUG_FLAGS) -c -o $@ $<

$(OUT)/%.o: %.cc globals.h
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(INCLUDES) $(DEBUG_FLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: install

all: $(TARGET)

install: $(TARGET)
	mkdir $(STAGING)
	$(NACL_SDK_ROOT)/tools/create_nmf.py -t glibc \
		-L $(NACL_TOOLCHAIN_ROOT)/$(ARCH)/lib \
		$(EXTRA_GEN_NMF_ARGS) \
		-s $(STAGING) -o $(STAGING)/jumpnbump.nmf $(TARGET)
	cp app/jumpnbump.html $(STAGING)/

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
	rm -rf $(STAGING)

doc:
	rman jumpnbump.6 -f HTML >jumpnbump.html

tools/genres: tools/genres.c
	$(HOST_CC) -o tools/genres tools/genres.c

resources.c: data/jumpbump.dat tools/genres
	tools/genres data/jumpbump.dat resources.c
