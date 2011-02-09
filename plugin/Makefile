# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the PI Generator example.

.PHONY: all clean

CCFILES = pi_generator.cc pi_generator_module.cc

OBJECTS_X86_64 = pi_generator_x86_64.o pi_generator_module_x86_64.o

CPP=$(NACL_SDK_ROOT)/toolchain/linux_x86/bin/nacl64-g++

INCLUDES = -I$(NACL_SDK_ROOT) -I$(SDL_PREFIX)/include/SDL
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lgoogle_nacl_platform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

CFLAGS=-W -Wall

# all: pi_generator_x86_64.nexe
# all: pi_generator.nmf pi_generator_dbg.nmf
all: $(OBJECTS_X86_64)

%_x86_32.o: %.c
	$(CC) $(CFLAGS) -m32 $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%_x86_32.o: %.cc
	$(CPP) $(CFLAGS) -m32 $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%_x86_64.o: %.c
	$(CC) $(CFLAGS) -m64 $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%_x86_64.o: %.cc
	$(CPP) $(CFLAGS) -m64 $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<


pi_generator.nmf: pi_generator_x86_32.nexe pi_generator_x86_64.nexe
	@echo "Creating pi_generator.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 pi_generator_x86_64.nexe --x86-32 pi_generator_x86_32.nexe

pi_generator_dbg.nmf: pi_generator_x86_32_dbg.nexe pi_generator_x86_64_dbg.nexe
	@echo "Creating pi_generator_dbg.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 pi_generator_x86_64_dbg.nexe \
	 --x86-32 pi_generator_x86_32_dbg.nexe

pi_generator_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

pi_generator_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

pi_generator_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

pi_generator_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.nmf *.o *.obj *.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: pi_generator.nmf
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
