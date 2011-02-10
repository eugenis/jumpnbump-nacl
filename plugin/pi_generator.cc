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
const int kMaxPointCount = 1000000000;  // The total number of points to draw.
const uint32_t kOpaqueColorMask = 0xff000000;  // Opaque pixels.
const uint32_t kRedMask = 0xff0000;
const uint32_t kBlueMask = 0xff;
const uint32_t kRedShift = 16;
const uint32_t kBlueShift = 0;

// This is called by the brower when the 2D context has been flushed to the
// browser window.
void FlushCallback(void* data, int32_t result) {
  (void)result;
  static_cast<pi_generator::PiGenerator*>(data)->set_flush_pending(false);
}
}  // namespace

namespace pi_generator {

// A small helper RAII class that implementes a scoped pthread_mutex lock.
class ScopedMutexLock {
 public:
  explicit ScopedMutexLock(pthread_mutex_t* mutex) : mutex_(mutex) {
    if (pthread_mutex_lock(mutex_) != kPthreadMutexSuccess) {
      mutex_ = NULL;
    }
  }
  ~ScopedMutexLock() {
    if (mutex_)
      pthread_mutex_unlock(mutex_);
  }
  bool is_valid() const {
    return mutex_ != NULL;
  }
 private:
  pthread_mutex_t* mutex_;  // Weak reference.
};

// A small helper RAII class used to acquire and release the pixel lock.
class ScopedPixelLock {
 public:
  explicit ScopedPixelLock(PiGenerator* image_owner)
      : image_owner_(image_owner), pixels_(image_owner->LockPixels()) {}

  ~ScopedPixelLock() {
    pixels_ = NULL;
    image_owner_->UnlockPixels();
  }

  uint32_t* pixels() const {
    return pixels_;
  }
 private:
  PiGenerator* image_owner_;  // Weak reference.
  uint32_t* pixels_;  // Weak reference.

  ScopedPixelLock();  // Not implemented, do not use.
};

PiGenerator::PiGenerator(PP_Instance instance)
    : pp::Instance(instance),
      graphics_2d_context_(NULL),
      pixel_buffer_(NULL),
      flush_pending_(false),
      quit_(false),
      compute_pi_thread_(0),
      pi_(0.0) {

  pp::Instance* inst = static_cast<pp::Instance*>(this);
  printf("pp_instance %p\n", (void*)inst);
  SDL_NACL_SetInstance(reinterpret_cast<void*>(inst));
  pthread_mutex_init(&pixel_buffer_mutex_, NULL);
}

PiGenerator::~PiGenerator() {
  quit_ = true;
  if (compute_pi_thread_) {
    pthread_join(compute_pi_thread_, NULL);
  }
  // DestroyContext();
  // The ComputePi() thread should be gone by now, so there is no need to
  // acquire the mutex for |pixel_buffer_|.
  delete pixel_buffer_;
  pthread_mutex_destroy(&pixel_buffer_mutex_);
}

void PiGenerator::DidChangeView(const pp::Rect& position,
                                const pp::Rect& clip) {
  if (position.size().width() == width() &&
      position.size().height() == height())
    return;  // Size didn't change, no need to update anything.

  printf("didchangeview, instance %p %p\n", this, static_cast<pp::Instance*>(this));
  if (!pixel_buffer_) {
    int lval = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    assert(lval >= 0);
    pthread_create(&compute_pi_thread_, NULL, ComputePi, this);
  }
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

uint32_t* PiGenerator::LockPixels() {
  void* pixels = NULL;
  // Do not use a ScopedMutexLock here, since the lock needs to be held until
  // the matching UnlockPixels() call.
  if (pthread_mutex_lock(&pixel_buffer_mutex_) == kPthreadMutexSuccess) {
    if (pixel_buffer_ != NULL && !pixel_buffer_->is_null()) {
      pixels = pixel_buffer_->data();
    }
  }
  return reinterpret_cast<uint32_t*>(pixels);
}

void PiGenerator::UnlockPixels() const {
  pthread_mutex_unlock(&pixel_buffer_mutex_);
}

pp::Var PiGenerator::Paint() {
  ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
  if (!scoped_mutex.is_valid()) {
    return pp::Var(kInvalidPiValue);
  }
  FlushPixelBuffer();
  return pp::Var(pi());
}

void PiGenerator::CreateContext(const pp::Size& size) {
  ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
  if (!scoped_mutex.is_valid()) {
    return;
  }
  if (IsContextValid())
    return;
  graphics_2d_context_ = new pp::Graphics2D(this, size, false);
  if (!BindGraphics(*graphics_2d_context_)) {
    printf("Couldn't bind the device context\n");
  }
}

void PiGenerator::DestroyContext() {
  ScopedMutexLock scoped_mutex(&pixel_buffer_mutex_);
  if (!scoped_mutex.is_valid()) {
    return;
  }
  if (!IsContextValid())
    return;
  delete graphics_2d_context_;
  graphics_2d_context_ = NULL;
}

void PiGenerator::FlushPixelBuffer() {
  if (!IsContextValid())
    return;
  // Note that the pixel lock is held while the buffer is copied into the
  // device context and then flushed.
  graphics_2d_context_->PaintImageData(*pixel_buffer_, pp::Point());
  if (flush_pending())
    return;
  set_flush_pending(true);
  graphics_2d_context_->Flush(pp::CompletionCallback(&FlushCallback, this));
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
  jumpnbump_main(3, (char**)argv);
  return NULL;
}
// void* PiGenerator::ComputePi(void* param) {
//   int count = 0;  // The number of points put inside the inscribed quadrant.
//   unsigned int seed = 1;
//   PiGenerator* pi_generator = static_cast<PiGenerator*>(param);
//   srand(seed);
//   for (int i = 1; i <= kMaxPointCount && !pi_generator->quit(); ++i) {
//     ScopedPixelLock scoped_pixel_lock(pi_generator);
//     uint32_t* pixel_bits = scoped_pixel_lock.pixels();
//     if (pixel_bits == NULL) {
//       // Note that if the pixel buffer never gets initialized, this won't ever
//       // paint anything.  Which is probably the right thing to do.  Also, this
//       // clause means that the image will not get the very first few Pi dots,
//       // since it's possible that this thread starts before the pixel buffer is
//       // initialized.
//       continue;
//     }
//     double x = static_cast<double>(rand_r(&seed)) / RAND_MAX;
//     double y = static_cast<double>(rand_r(&seed)) / RAND_MAX;
//     double distance = sqrt(x * x + y * y);
//     int px = x * pi_generator->width();
//     int py = (1.0 - y) * pi_generator->height();
//     uint32_t color = pixel_bits[pi_generator->width() * py + px];
//     if (distance < 1.0) {
//       // Set color to blue.
//       ++count;
//       pi_generator->pi_ = 4.0 * count / i;
//       color += 4 << kBlueShift;
//       color &= kBlueMask;
//     } else {
//       // Set color to red.
//       color += 4 << kRedShift;
//       color &= kRedMask;
//     }
//     pixel_bits[pi_generator->width() * py + px] = color | kOpaqueColorMask;
//   }
//   return 0;
// }

}  // namespace pi_generator
