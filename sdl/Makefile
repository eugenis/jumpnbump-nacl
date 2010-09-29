NACL_CC=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl64-gcc
SDL_CFLAGS = -I$(SDL_DIR)/include
CFLAGS = -Wall -ansi -pedantic -g -O0 -ffast-math -funroll-loops \
	-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DNDEBUG \
	-I. -I.. $(SDL_CFLAGS) -DUSE_SDL #-DUSE_NET

OBJS = gfx.o interrpt.o sound.o input.o
TARGET = ../sdl.a

all: $(TARGET)

$(TARGET): $(OBJS)
	ar cru $(TARGET) $(OBJS)
	ranlib $(TARGET)

%.o: %.c
	$(NACL_CC) $(CFLAGS) -m64 $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
