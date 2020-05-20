#pragma once
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

#include "aligned-buffer.h"
#include "shadow-stack.h"
#include "statistics.h"

#include <dr_api.h>
#include <hashtable.h>

#include <atomic>
#include <type_traits>

namespace drace {

/**
 * \brief Per Thread data (thread-private)
 */
class ShadowThreadState {
 public:
  byte* buf_ptr;
  ptr_int_t buf_end;

  /// Represents the detector state.
  byte enabled{true};
  /// inverse of flush pending, jmpecxz
  std::atomic<byte> no_flush{false};
  /// local sampling state
  int sampling_pos = 0;

  thread_id_t tid;

  /// track state of detector (count nr of disable / enable calls)
  uintptr_t event_cnt{0};

  /// begin of this threads stack range
  uintptr_t appstack_beg{0x0};
  /// end of this threads stack range
  uintptr_t appstack_end{0x0};

  /**
   * as the detector cannot allocate TLS,
   * use this ptr for per-thread data in detector */
  void* detector_data{nullptr};

  /// buffer containing memory accesses
  AlignedBuffer<byte, 64> mem_buf;

  /// book-keeping of active mutexes
  hashtable_t mutex_book;

  /// ShadowStack
  ShadowStack stack;

  /// Statistics
  Statistics stats;

 public:
  explicit ShadowThreadState(void* drcontext);
  ~ShadowThreadState();
};

// this type trait is not correctly implemented by all compilers / stdlibs
// static_assert(std::is_standard_layout<ShadowThreadState>::value,
//              "ShadowThreadState has to be POD to be safely accessed via "
//              "inline assembly");

}  // namespace drace
