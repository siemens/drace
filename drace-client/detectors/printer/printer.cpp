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
#define PRINTER_EXPORT __declspec(dllexport)
#else
#define PRINTER_EXPORT
#endif

namespace drace {
namespace detector {
/// Fake detector that stubs the \ref Detector interface
class Printer : public Detector {
 public:
  virtual bool init(int argc, const char** argv, Callback rc_clb) {
    printf("init\n");
    return true;
  }

  virtual void finalize() { printf("%p | finalize       |\n", nullptr); }

  virtual void map_shadow(void* startaddr, size_t size_in_bytes) {
    printf("%p | map_shadow     | addr=%p size=%zu\n", nullptr, startaddr,
           size_in_bytes);
  };

  virtual void func_enter(tls_t tls, void* pc) {
    printf("%p | func_enter     | pc=%p\n", tls, pc);
  }

  virtual void func_exit(tls_t tls) { printf("%p | func_exit      |\n", tls); }

  virtual void acquire(tls_t tls, void* mutex, int recursive, bool write) {
    printf("%p | acquire        | m=%p rec=%i w=%i\n", tls, mutex, recursive,
           write);
  }

  virtual void release(tls_t tls, void* mutex, bool write) {
    printf("%p | release        | m=%p, w=%i\n", tls, mutex, write);
  }

  virtual void happens_before(tls_t tls, void* identifier) {
    printf("%p | happens_before | %p\n", tls, identifier);
  }

  virtual void happens_after(tls_t tls, void* identifier) {
    printf("%p | happens_after  | %p\n", tls, identifier);
  }

  virtual void read(tls_t tls, void* pc, void* addr, size_t size) {
    printf("%p | read           | pc=%p, addr=%p, s=%zu\n", tls, pc, addr,
           size);
  }

  virtual void write(tls_t tls, void* pc, void* addr, size_t size) {
    printf("%p | write          | pc=%p, addr=%p, s=%zu\n", tls, pc, addr,
           size);
  }

  virtual void allocate(tls_t tls, void* pc, void* addr, size_t size) {
    printf("%p | allocate       | pc=%p, addr=%p, s=%zu\n", tls, pc, addr,
           size);
  }

  virtual void deallocate(tls_t tls, void* addr) {
    printf("%p | deallocate     | addr=%p\n", tls, addr);
  }

  virtual void fork(tid_t parent, tid_t child, tls_t* tls) {
    *tls = (void*)(0xFF0000ull + child);
    printf("%p | fork           | par=%i, child=%i\n", *tls, parent, child);
  }

  virtual void join(tid_t parent, tid_t child) {
    printf("%p | join           | par=%i, child=%i\n", nullptr, parent, child);
  }

  virtual void detach(tls_t tls, tid_t thread_id) {
    printf("%p | detach         | tid=%i\n", tls, thread_id);
  };

  virtual void finish(tls_t tls, tid_t thread_id) {
    printf("%p | finish         | tid=%i\n", tls, thread_id);
  };

  virtual const char* name() {
    printf("%p | name           |\n", nullptr);
    return "Dummy";
  }

  virtual const char* version() {
    printf("%p | version        |\n", nullptr);
    return "1.0.0";
  }
};
}  // namespace detector
}  // namespace drace

extern "C" PRINTER_EXPORT Detector* CreateDetector() {
  return new drace::detector::Printer();
}

// extern "C" PRINTER_EXPORT void Read_Nvrt(Detector& detector, void* tls,
//                                          void* pc, void* addr, size_t size) {
//   detector.read(tls, pc, addr, size);
// }
