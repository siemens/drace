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

#include <fstream>
#include <iostream>
#include <string>

#include <detector/Detector.h>
#include <dr_api.h>
#include <ipc/ExtsanData.h>

#ifdef WINDOWS
#define TRACEBINARY_EXPORT __declspec(dllexport)
#else
#define TRACEBINARY_EXPORT
#endif

using namespace ipc::event;

namespace drace {
namespace detector {
/// Fake detector that traces all calls to the \ref Detector interface
class TraceBinary : public Detector {
 private:
  void* iolock;
  std::fstream file;

 public:
  TraceBinary() {
    iolock = dr_mutex_create();
    file = std::fstream("trace.bin", std::ios::out | std::ios::binary);
  }

  virtual bool init(int argc, const char** argv, Callback rc_clb) {
    return true;
  }

  virtual void finalize() {
    dr_mutex_destroy(iolock);
    file.close();
  }

  virtual void map_shadow(void* startaddr, size_t size_in_bytes){};

  virtual void func_enter(tls_t tls, void* pc) {
    ipc::event::BufferEntry buf{Type::FUNCENTER};
    buf.payload.funcenter = {(uint32_t)(uintptr_t)tls, (uintptr_t)pc};
    write_log_sync(buf);
  }

  virtual void func_exit(tls_t tls) {
    ipc::event::BufferEntry buf{Type::FUNCEXIT};
    buf.payload.funcexit = {(uint32_t)(uintptr_t)tls};
    write_log_sync(buf);
  }

  virtual void acquire(tls_t tls, void* mutex, int recursive, bool write) {
    ipc::event::BufferEntry buf{Type::ACQUIRE};
    buf.payload.mutex = {(uint32_t)(uintptr_t)tls, (uintptr_t)mutex,
                         (int)recursive, write, true};
    write_log_sync(buf);
  }

  virtual void release(tls_t tls, void* mutex, bool write) {
    ipc::event::BufferEntry buf{Type::RELEASE};
    buf.payload.mutex = {(uint32_t)(uintptr_t)tls, (uintptr_t)mutex, (int)0,
                         write, false};
    write_log_sync(buf);
  }

  virtual void happens_before(tls_t tls, void* identifier) {
    ipc::event::BufferEntry buf{Type::HAPPENSBEFORE};
    buf.payload.happens = {(uint32_t)(uintptr_t)tls, (uintptr_t)identifier};
    write_log_sync(buf);
  }

  virtual void happens_after(tls_t tls, void* identifier) {
    ipc::event::BufferEntry buf{Type::HAPPENSAFTER};
    buf.payload.happens = {(uint32_t)(uintptr_t)tls, (uintptr_t)identifier};
    write_log_sync(buf);
  }

  virtual void read(tls_t tls, void* pc, void* addr, size_t size) {
    ipc::event::BufferEntry buf{Type::MEMREAD};
    buf.payload.memaccess = {(uint32_t)(uintptr_t)tls, (uintptr_t)pc,
                             (uintptr_t)addr, (uintptr_t)size};
    write_log_sync(buf);
  }

  virtual void write(tls_t tls, void* pc, void* addr, size_t size) {
    ipc::event::BufferEntry buf{Type::MEMWRITE};
    buf.payload.memaccess = {(uint32_t)(uintptr_t)tls, (uintptr_t)pc,
                             (uintptr_t)addr, (uintptr_t)size};
    write_log_sync(buf);
  }

  virtual void allocate(tls_t tls, void* pc, void* addr, size_t size) {
    ipc::event::BufferEntry buf{Type::ALLOCATION};
    buf.payload.allocation = {(uint32_t)(uintptr_t)tls, (uintptr_t)pc,
                              (uintptr_t)addr, (uintptr_t)size};
    write_log_sync(buf);
  }

  virtual void deallocate(tls_t tls, void* addr) {
    ipc::event::BufferEntry buf{Type::FREE};
    buf.payload.allocation = {(uint32_t)(uintptr_t)tls, (uintptr_t)0x0,
                              (uintptr_t)addr, (uintptr_t)0x0};
    write_log_sync(buf);
  }

  virtual void fork(tid_t parent, tid_t child, tls_t* tls) {
    *tls = (void*)((uintptr_t)child);
    ipc::event::BufferEntry buf{Type::FORK};
    buf.payload.forkjoin = {(uint32_t)(uintptr_t)parent, (uint32_t)child};
    write_log_sync(buf);
  }

  virtual void join(tid_t parent, tid_t child) {
    ipc::event::BufferEntry buf{Type::JOIN};
    buf.payload.forkjoin = {(uint32_t)(uintptr_t)parent, (uint32_t)child};
    write_log_sync(buf);
  }

  virtual void detach(tls_t tls, tid_t thread_id) {
    ipc::event::BufferEntry buf{Type::DETACH};
    buf.payload.detachfinish = {(uint32_t)(uintptr_t)tls};
    write_log_sync(buf);
  };

  virtual void finish(tls_t tls, tid_t thread_id) {
    ipc::event::BufferEntry buf{Type::FINISH};
    buf.payload.detachfinish = {(uint32_t)(uintptr_t)tls};
    write_log_sync(buf);
  };

  virtual const char* name() { return "Dummy"; }

  virtual const char* version() { return "1.0.0"; }

 private:
  void write_log_sync(const ipc::event::BufferEntry& event) {
    dr_mutex_lock(iolock);
    file.write((char*)&event, sizeof(ipc::event::BufferEntry));
    dr_mutex_unlock(iolock);
  }
};
}  // namespace detector
}  // namespace drace

extern "C" TRACEBINARY_EXPORT Detector* CreateDetector() {
  return new drace::detector::TraceBinary();
}

// extern "C" TRACEBINARY_EXPORT void Read_Nvrt(Detector& detector, void* tls,
//                                              void* pc, void* addr,
//                                              size_t size) {
//   detector.read(tls, pc, addr, size);
// }
