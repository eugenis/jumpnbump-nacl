// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/size.h>

#include <nacl-mounts/base/KernelProxy.h>
#include <nacl-mounts/net/SocketSubSystem.h>

#include <SDL_video.h>
extern "C" {
extern int jumpnbump_main(int argc, const char *argv[]);
}
#include <SDL.h>
#include <SDL_nacl.h>

class PluginInstance : public pp::Instance {
 public:
  explicit PluginInstance(PP_Instance instance) : pp::Instance(instance),
						  sdl_main_thread_(0),
						  width_(0),
						  height_(0) {
    RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
    RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);
    proxy_ = KernelProxy::KPInstance();
    proxy_->SetSocketSubSystem(new SocketSubSystem(this));
  }

  ~PluginInstance() {
    if (sdl_main_thread_) {
      pthread_join(sdl_main_thread_, NULL);
    }
  }

  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip) {
    printf("did change view, new %dx%d, old %dx%d\n",
	   position.size().width(), position.size().height(),
	   width_, height_);

    if (position.size().width() == width_ &&
	position.size().height() == height_)
      return;  // Size didn't change, no need to update anything.

    if (sdl_thread_started_ == false) {
      width_ = position.size().width();
      height_ = position.size().height();

      SDL_NACL_SetInstance(pp_instance(), width_, height_);
      // It seems this call to SDL_Init is required. Calling from
      // sdl_main() isn't good enough.
      // Perhaps it must be called from the main thread?
      int lval = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
      assert(lval >= 0);
      if (0 == pthread_create(&sdl_main_thread_, NULL, sdl_thread, this)) {
	sdl_thread_started_ = true;
      }
    }
  }

  bool HandleInputEvent(const pp::InputEvent& event) {
    SDL_NACL_PushEvent(event);
    return true;
  }

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    is_server = false;
    for (int i = 0; i < argc; ++i) {
      if (strcmp(argn[i], "role") == 0) {
	is_server = (strcmp(argv[i], "server") == 0);
      }
    }
    return true;
  }

 private:
  bool sdl_thread_started_;
  pthread_t sdl_main_thread_;
  int width_;
  int height_;
  KernelProxy* proxy_;
  bool is_server;

  static void* sdl_thread(void* param) {
    PluginInstance* self = (PluginInstance*)param;
    static char const * argv_server [] = {"jumpnbump", "-scaleup", "-fullscreen", "-server", "1"};
    static char const * argv_client [] = {"jumpnbump", "-scaleup", "-fullscreen", "-connect", "localhost"};
    jumpnbump_main(5, (const char**)(self->is_server ? argv_server : argv_client));
    return NULL;
  }
};

class PepperModule : public pp::Module {
 public:
  // Create and return a PluginInstanceInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new PluginInstance(instance);
  }
};

namespace pp {
  Module* CreateModule() {
    return new PepperModule();
  }
}  // namespace pp
