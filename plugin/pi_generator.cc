// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "pi_generator.h"

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/var.h>
#include <stdio.h>
#include <stdlib.h>

#include <cassert>
#include <cmath>
#include <cstring>

extern "C" {
#include <SDL_video.h>
extern int jumpnbump_main(int argc, char *argv[]);

  // temporary hacks
#include <SDL.h>
#include <SDL_nacl.h>
}

namespace {
const int kPthreadMutexSuccess = 0;
const char* const kPaintMethodId = "paint";
const double kInvalidPiValue = -1.0;

// This is called by the brower when the 2D context has been flushed to the
// browser window.
void FlushCallback(void* data, int32_t result) {
  (void)result;
  static_cast<pi_generator::PiGenerator*>(data)->set_flush_pending(false);
}
}  // namespace

namespace pi_generator {

PiGenerator::PiGenerator(PP_Instance instance)
    : pp::Instance(instance),
      graphics_2d_context_(NULL),
      flush_pending_(false),
      quit_(false),
      compute_pi_thread_(0),
      width_(0),
      height_(0),
      pi_(0.0) {

}

PiGenerator::~PiGenerator() {
  quit_ = true;
  if (compute_pi_thread_) {
    pthread_join(compute_pi_thread_, NULL);
  }
  // DestroyContext();
}

void PiGenerator::DidChangeView(const pp::Rect& position,
                                const pp::Rect& clip) {

  printf("did change view, new %dx%d, old %dx%d\n", position.size().width(), position.size().height(),
      width_, height_);

  if (width_ && height_)
    return;

  if (position.size().width() == width_ &&
      position.size().height() == height_)
    return;  // Size didn't change, no need to update anything.

  width_ = position.size().width();
  height_ = position.size().height();

  printf("didchangeview, instance %p %p\n", this, static_cast<pp::Instance*>(this));
  SDL_NACL_SetInstance(pp_instance(), width_, height_);
  int lval = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  assert(lval >= 0);
  pthread_create(&compute_pi_thread_, NULL, ComputePi, this);
}

pp::Var PiGenerator::GetInstanceObject() {
  PiGeneratorScriptObject* script_object = new PiGeneratorScriptObject(this);
  return pp::Var(this, script_object);
}

bool PiGenerator::HandleInputEvent(const PP_InputEvent& event) {
  SDL_NACL_PushEvent(&event);
  return true;
}

bool PiGenerator::Init(uint32_t argc, const char* argn[], const char* argv[]) {
  // pthread_create(&compute_pi_thread_, NULL, ComputePi, this);
  return true;
}

pp::Var PiGenerator::Paint() {
  return pp::Var(pi());
}

bool PiGenerator::PiGeneratorScriptObject::HasMethod(
    const pp::Var& method,
    pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  return method_name == kPaintMethodId;
}

pp::Var PiGenerator::PiGeneratorScriptObject::Call(
    const pp::Var& method,
    const std::vector<pp::Var>& args,
    pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  if (app_instance_ != NULL && method_name == kPaintMethodId) {
    return app_instance_->Paint();
  }
  return pp::Var();
}

void* PiGenerator::ComputePi(void* param) {
  // static char const * argv [] = {"-fireworks", "-fireworks","-fireworks" };
  // jumpnbump_main(2, argv, &render, &poll_event, param);
  static char const * argv [] = {"jumpnbump", "-scaleup", "-fullscreen"};
  printf("Starting jumpnbump\n");
  jumpnbump_main(3, (char**)argv);
  printf("main() finished\n");
  return NULL;
}

}  // namespace pi_generator
