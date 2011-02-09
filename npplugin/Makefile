CCFILES = npn_bridge.cc \
          npp_gate.cc \
          pi_generator.cc \
          pi_generator_module.cc \
          scripting_bridge.cc

OBJECTS = $(CCFILES:%.cc=%.o)

CPP=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl64-g++
CFLAGS = -Wall -Wno-long-long -pthread -DXP_UNIX -Werror
INCLUDES = -I$(SDL_PREFIX)/include/SDL

all: $(OBJECTS)

%.o: %.cc
	$(CPP) $(CFLAGS) -m64 $(INCLUDES) -c -o $@ $<

clean:
	-rm $(OBJECTS)
