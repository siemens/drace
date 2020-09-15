#pragma once
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

#include "detector/Detector.h"
#include "func_def.h"
#include <array>

namespace drace {
/**
 * \brief shadow stack implementation for stack-tracing
 *
 * This class implements a shadow stack, used for
 * getting stack traces for race detection
 * Whenever a call has been detected, the memory-refs buffer is
 * flushed as all memory-refs happend in the current function.
 * This avoids storing a stack-trace per memory-entry.
 */
class ShadowStack {
 public:
 private:
  /**
   * \brief maximum number of stack entries.
   * That's one less than in the detector,
   * as there the last element represents the
   * racy pc
   */
  static constexpr int max_size = Detector::max_stack_size - 1;

  std::array<void*, max_size> _data;
  int _entries{0};
  Detector* _detector{nullptr};

 public:
  ShadowStack() = default;
  explicit ShadowStack(Detector* det) : _detector(det) {}

  inline void bindDetector(Detector* det) { _detector = det; }

  /// return true if the stack is empty
  inline bool isEmpty() const { return _entries == 0; }

  /**
   * \brief Push a pc on the stack.
   * \note more pushes than stack-size are allowed, but ignored
   */
  inline void push(void* addr, void* det_data) {
    if (_entries >= max_size) return;

    if (FuncEnter_Nvrt_Ptr) {
      FuncEnter_Nvrt_Ptr((void*)_detector, det_data, addr);
    } else {
      _detector->func_enter(det_data, addr);
    }
    // _detector->func_enter(det_data, addr);
    _data[_entries++] = addr;
  }

  /**
   * \brief pop the top element of the stack
   * \note more pops than stack-elements are not allowed
   *       (caller is responsible)
   */
  inline void* pop(void* det_data) {
#ifdef DYNAMORIO_API
    DR_ASSERT(_entries > 0);
#endif
    --_entries;

    if (FuncExit_Nvrt_Ptr) {
      FuncExit_Nvrt_Ptr((void*)_detector, det_data);
    } else {
      _detector->func_exit(det_data);
    }
      // _detector->func_exit(det_data);

    return _data[_entries];
  }

  inline size_t size() const { return _entries; }

  static constexpr size_t maxSize() {
    // this getter is to work around ODR limitations
    // in C++11
    return max_size;
  }
};  // namespace drace
}  // namespace drace
