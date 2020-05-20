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
#ifndef _DR_LOCK_H
#define _DR_LOCK_H

#include "dr_api.h"

/**
 * \brief Mutex implementation which internally uses DynamoRIO mutexes.
 *
 * The class implements the \ref std::shared_timed_mutex interface
 */
class DrLock {
 private:
  void* this_lock;

 public:
  inline explicit DrLock() { this_lock = dr_rwlock_create(); }

  inline ~DrLock() { dr_rwlock_destroy(this_lock); }

  inline void lock_shared() { dr_rwlock_read_lock(this_lock); }

  /**
   * \brief Not implemented
   */
  inline bool try_lock_shared() {
    // no dynamrio api function available
    return false;
  }

  inline void unlock_shared() { dr_rwlock_read_unlock(this_lock); }

  inline void lock() { dr_rwlock_write_lock(this_lock); }

  inline void unlock() { dr_rwlock_write_unlock(this_lock); }

  inline bool try_lock() { return dr_rwlock_write_trylock(this_lock); }
};

#endif  // !_DR_LOCK_H
