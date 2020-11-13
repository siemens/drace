/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <detector/Detector.h>

class ThreadStatePy {
 public:
  ThreadStatePy(Detector::tls_t tls, Detector* det) : _tls{tls}, _det{det} {}

  void acquire(size_t mutex, int recursive, bool write) {
    _det->acquire(_tls, reinterpret_cast<void*>(mutex), recursive, write);
  }
  void release(size_t mutex, bool write) {
    _det->release(_tls, reinterpret_cast<void*>(mutex), write);
  }
  void happens_before(size_t identifier) {
    _det->happens_before(_tls, reinterpret_cast<void*>(identifier));
  }
  void happens_after(size_t identifier) {
    _det->happens_after(_tls, reinterpret_cast<void*>(identifier));
  }
  void func_enter(size_t pc) {
    _det->func_enter(_tls, reinterpret_cast<void*>(pc));
  }
  void func_exit() { _det->func_exit(_tls); }
  void read(size_t pc, size_t addr, size_t size) {
    _det->read(_tls, reinterpret_cast<void*>(pc), reinterpret_cast<void*>(addr),
               size);
  }
  void write(size_t pc, size_t addr, size_t size) {
    _det->write(_tls, reinterpret_cast<void*>(pc),
                reinterpret_cast<void*>(addr), size);
  }

 private:
  Detector::tls_t _tls;
  Detector* _det;
};
