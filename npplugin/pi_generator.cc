// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "pi_generator.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nacl/nacl_imc.h>
#include <nacl/nacl_npapi.h>
#include <nacl/npapi_extensions.h>
#include <nacl/npruntime.h>

#include "scripting_bridge.h"

using pi_generator::ScriptingBridge;
extern NPDevice* NPN_AcquireDevice(NPP instance, NPDeviceID device);

extern "C" {
#include <SDL_video.h>
extern int jumpnbump_main(int argc, char *argv[]);

  // temporary hacks
#include <SDL.h>
}

// This is called by the brower when the 2D context has been flushed to the
// browser window.
void FlushCallback(NPP instance, NPDeviceContext* context,
                   NPError err, void* user_data) {
}

namespace pi_generator {

PiGenerator::PiGenerator(NPP npp)
    : npp_(npp),
      scriptable_object_(NULL),
      window_(NULL),
      device2d_(NULL),
      quit_(false),
      thread_(0),
      pi_(0.0) {
  SDL_NACL_SetNPP((void*)npp);
  ScriptingBridge::InitializeIdentifiers();
}

PiGenerator::~PiGenerator() {
  quit_ = true;
  if (thread_) {
    pthread_join(thread_, NULL);
  }
  if (scriptable_object_) {
    NPN_ReleaseObject(scriptable_object_);
  }
}

NPObject* PiGenerator::GetScriptableObject() {
  if (scriptable_object_ == NULL) {
    scriptable_object_ =
      NPN_CreateObject(npp_, &ScriptingBridge::np_class);
  }
  if (scriptable_object_) {
    NPN_RetainObject(scriptable_object_);
  }
  return scriptable_object_;
}

NPError PiGenerator::SetWindow(NPWindow* window) {
  if (!window)
    return NPERR_NO_ERROR;

  printf("SetWindow ============================\n");
  if (!window_) { // just the first time
    int lval = SDL_Init(SDL_INIT_VIDEO);
    assert(lval >= 0);
    pthread_create(&thread_, NULL, jumpnbump_thread, this);
    window_ = window;
  }
  // return Paint() ? NPERR_NO_ERROR : NPERR_GENERIC_ERROR;
  return NPERR_NO_ERROR;
}

void PiGenerator::HandleEvent(NPPepperEvent* npevent) {
}

bool PiGenerator::Paint() {
  printf("Paint() stub\n");
  SDL_WM_SetCaption("Dear SDL, this is your chance to flush the NPDevice context.", NULL);
  return true;
}
    
  void* PiGenerator::jumpnbump_thread(void* param) {
    // static char const * argv [] = {"-fireworks", "-fireworks","-fireworks" };
    // jumpnbump_main(2, argv, &render, &poll_event, param);
    jumpnbump_main(0, NULL);
    return NULL;
  }


}  // namespace pi_generator

