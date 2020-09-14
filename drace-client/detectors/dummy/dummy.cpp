/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <string>

#include <detector/Detector.h>

#ifdef WINDOWS
#define DUMMY_EXPORT __declspec(dllexport)
#else
#define DUMMY_EXPORT
#endif

namespace drace {
namespace detector {
/// Fake detector that stubs the \ref Detector interface
class Dummy : public Detector {
 public:
  virtual bool init(int argc, const char** argv, Callback rc_clb) {
    return true;
  }

  virtual void finalize() {}

  virtual void map_shadow(void* startaddr, size_t size_in_bytes){};

  virtual void func_enter(tls_t tls, void* pc) {}

  virtual void func_exit(tls_t tls) {}

  virtual void acquire(tls_t tls, void* mutex, int recursive, bool write) {}

  virtual void release(tls_t tls, void* mutex, bool write) {}

  virtual void happens_before(tls_t tls, void* identifier) {}

  virtual void happens_after(tls_t tls, void* identifier) {}

  virtual void read(tls_t tls, void* pc, void* addr, size_t size) {}

  virtual void write(tls_t tls, void* pc, void* addr, size_t size) {}

  virtual void allocate(tls_t tls, void* pc, void* addr, size_t size) {}

  virtual void deallocate(tls_t tls, void* addr) {}

  virtual void fork(tid_t parent, tid_t child, tls_t* tls) {
    *tls = (void*)(42ull + child);
  }

  virtual void join(tid_t parent, tid_t child) {}

  virtual void detach(tls_t tls, tid_t thread_id){};

  virtual void finish(tls_t tls, tid_t thread_id){};

  virtual const char* name() { return "Dummy"; }

  virtual const char* version() { return "1.0.0"; }
};
}  // namespace detector
}  // namespace drace

extern "C" DUMMY_EXPORT Detector* CreateDetector() {
  return new drace::detector::Dummy();
}

// extern "C" DUMMY_EXPORT void Read_Nvrt(Detector& detector, void* tls, void* pc,
//                                        void* addr, size_t size) {
//   detector.read(tls, pc, addr, size);
// }
