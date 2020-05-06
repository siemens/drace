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

#include <atomic>
#include <mutex>
#include <thread>
#ifdef DEBUG
#include <iostream>
#endif

#ifdef HAVE_SSE2
#include <immintrin.h>  //_mm_pause
#endif

namespace ipc {
/**
 * \brief mutex implemented as a spinlock
 *
 * implements interface of \ref std::mutex
 *
 * \note only use this spinlock, if the critical section is short
 *       and the pressure on the lock is low
 *
 * \note this spinlock is cache-coherence friendly and
 *       has optimizations for hyper-threading CPUs
 */
class spinlock{
  std::atomic<bool> _locked{false};
 public:

  inline void lock() noexcept {
    for (int spin_count = 0; !try_lock(); ++spin_count) {
      if (spin_count < 16) {
#ifdef HAVE_SSE2
        // tell the CPU (not OS), that we are spin-waiting
        _mm_pause();
#endif
      } else {
        // avoid waisting CPU time in rare long-wait scenarios
        std::this_thread::yield();
      }
    }
  }

  inline bool try_lock() noexcept {
    // first check if lock is free, then try to exchange
    // the result tells if the value did change.
    // if not, the mutex has been locked in the meantime by another
    // thread, and we report that out try_lock was not successfull
    return !_locked.load(std::memory_order_relaxed) &&
           !_locked.exchange(true, std::memory_order_acquire);
  }

  inline void unlock() noexcept {
    _locked.store(false, std::memory_order_release);
  }
};
}  // namespace ipc
