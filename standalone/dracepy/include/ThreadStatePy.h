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

namespace dracepy {

/**
 * \brief Thread context of the \ref Detector interface
 *
 * This class is used to access all thread specific \ref Detector methods
 */
class ThreadStatePy {
 public:
  ThreadStatePy(Detector::tls_t tls, Detector* det) : _tls{tls}, _det{det} {}

  /// Wrapper around \ref Detector::acquire
  void acquire(size_t mutex, int recursive, bool write) {
    _det->acquire(_tls, reinterpret_cast<void*>(mutex), recursive, write);
  }
  /// Wrapper around \ref Detector::release
  void release(size_t mutex, bool write) {
    _det->release(_tls, reinterpret_cast<void*>(mutex), write);
  }
  /// Wrapper around \ref Detector::happens_before
  void happens_before(size_t identifier) {
    _det->happens_before(_tls, reinterpret_cast<void*>(identifier));
  }
  /// Wrapper around \ref Detector::happens_after
  void happens_after(size_t identifier) {
    _det->happens_after(_tls, reinterpret_cast<void*>(identifier));
  }
  /// Wrapper around \ref Detector::func_enter
  void func_enter(size_t pc) {
    _det->func_enter(_tls, reinterpret_cast<void*>(pc));
  }
  /// Wrapper around \ref Detector::func_exit
  void func_exit() { _det->func_exit(_tls); }
  /// Wrapper around \ref Detector::read
  void read(size_t pc, size_t addr, size_t size) {
    _det->read(_tls, reinterpret_cast<void*>(pc), reinterpret_cast<void*>(addr),
               size);
  }
  /// Wrapper around \ref Detector::write
  void write(size_t pc, size_t addr, size_t size) {
    _det->write(_tls, reinterpret_cast<void*>(pc),
                reinterpret_cast<void*>(addr), size);
  }

 private:
  Detector::tls_t _tls;
  Detector* _det;
};

}  // namespace dracepy
