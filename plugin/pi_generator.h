// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_PI_GENERATOR_H_
#define EXAMPLES_PI_GENERATOR_H_

#include <ppapi/cpp/graphics_2d.h>
#include <ppapi/cpp/image_data.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/size.h>
#include <pthread.h>

#include <map>
#include <vector>

namespace pi_generator {

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurence of the <embed> tag that has these
// attributes:
//     type="application/x-ppapi-nacl-srpc"
//     nexes="ARM: pi_generator_arm.nexe
//            ..."
// The Instance can return a ScriptableObject representing itself.  When the
// browser encounters JavaScript that wants to access the Instance, it calls
// the GetInstanceObject() method.  All the scripting work is done though
// the returned ScriptableObject.
class PiGenerator : public pp::Instance {
 public:
  explicit PiGenerator(PP_Instance instance);
  virtual ~PiGenerator();

  // Start up the ComputePi() thread.
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

  // Update the graphcs context to the new size, and regnerate |pixel_buffer_|
  // to fit the new size as well.
  virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);

  // The pp::Var takes over ownership of the returned script object.
  virtual pp::Var GetInstanceObject();

  virtual bool HandleInputEvent(const PP_InputEvent& event);

  // Return a pointer to the pixels represented by |pixel_buffer_|.  When this
  // method returns, the underlying |pixel_buffer_| object is locked.  This
  // call must have a matching UnlockPixels() or various threading errors
  // (e.g. deadlock) will occur.
  uint32_t* LockPixels();
  // Release the image lock acquired by LockPixels().
  void UnlockPixels() const;

  // Flushes its contents of |pixel_buffer_| to the 2D graphics context.  The
  // ComputePi() thread fills in |pixel_buffer_| pixels as it computes Pi.
  // This method is called in response to the "paint()" method being called
  // from JavaScript.  Returns the current value of pi as computed by the
  // Monte Carlo method.
  pp::Var Paint();

  bool quit() const {
    return quit_;
  }

  // |pi_| is computed in the ComputePi() thread.
  double pi() const {
    return pi_;
  }

  int width() const {
    return pixel_buffer_ ? pixel_buffer_->size().width() : 0;
  }
  int height() const {
    return pixel_buffer_ ? pixel_buffer_->size().height() : 0;
  }

  // Indicate whether a flush is pending.  This can only be called from the
  // main thread; it is not thread safe.
  bool flush_pending() const {
    return flush_pending_;
  }
  void set_flush_pending(bool flag) {
    flush_pending_ = flag;
  }

 private:
  // This class exposes the scripting interface for this NaCl module.  The
  // HasMethod method is called by the browser when executing a method call on
  // the |piGenerator| object (see, e.g. the paint() function in
  // pi_generator.html).  The name of the JavaScript function (e.g. "paint") is
  // passed in the |method| paramter as a string pp::Var.  If HasMethod()
  // returns |true|, then the browser will call the Call() method to actually
  // invoke the method.
  class PiGeneratorScriptObject : public pp::deprecated::ScriptableObject {
   public:
    explicit PiGeneratorScriptObject(PiGenerator* app_instance)
        : pp::deprecated::ScriptableObject(),
          app_instance_(app_instance) {}
    virtual ~PiGeneratorScriptObject() {}
    // Return |true| if |method| is one of the exposed method names.
    virtual bool HasMethod(const pp::Var& method, pp::Var* exception);

    // Invoke the function associated with |method|.  The argument list passed
    // in via JavaScript is marshaled into a vector of pp::Vars.  None of the
    // functions in this example take arguments, so this vector is always empty.
    virtual pp::Var Call(const pp::Var& method,
                         const std::vector<pp::Var>& args,
                         pp::Var* exception);
   private:
    PiGenerator* app_instance_;  // weak reference.
  };

  // Create and initialize the 2D context used for drawing.
  void CreateContext(const pp::Size& size);
  // Destroy the 2D drawing context.
  void DestroyContext();
  // Push the pixels to the browser, then attempt to flush the 2D context.  If
  // there is a pending flush on the 2D context, then update the pixels only
  // and do not flush.
  void FlushPixelBuffer();

  bool IsContextValid() const {
    return graphics_2d_context_ != NULL;
  }

  mutable pthread_mutex_t pixel_buffer_mutex_;
  pp::Graphics2D* graphics_2d_context_;
  pp::ImageData* pixel_buffer_;
  bool flush_pending_;
  bool quit_;
  pthread_t compute_pi_thread_;
  double pi_;

  // ComputePi() estimates Pi using Monte Carlo method and it is executed by a
  // separate thread created in SetWindow(). ComputePi() puts kMaxPointCount
  // points inside the square whose length of each side is 1.0, and calculates
  // the ratio of the number of points put inside the inscribed quadrant divided
  // by the total number of random points to get Pi/4.
  static void* ComputePi(void* param);
};

}  // namespace pi_generator

#endif  // EXAMPLES_PI_GENERATOR_H_
