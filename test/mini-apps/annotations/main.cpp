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

#include <ipc/spinlock.h>
#include <iostream>
#include <mutex>  // for lock_guard
#include <thread>
#include <vector>

// Set this define to enable annotations.
// we do that using CMake to cross test in
// integration tests
//#define DRACE_ANNOTATION
#include "../../../drace-client/include/annotations/drace_annotation.h"

/**
\brief This code serves as a test for client annotations
*/

/// number of increment repetitions
constexpr int NUM_INCREMENTS = 1000;
/// wait n microseconds between read and write
constexpr int WAIT_US = 200;

/**
 * Wait a configurable time to enforce a data-race
 */
inline void wait() {
  std::this_thread::sleep_for(std::chrono::microseconds(WAIT_US));
}

/**
 * Racy-increment a value but exclude this section
 * from analysis
 */
void inc(int* v) {
  for (int i = 0; i < NUM_INCREMENTS; ++i) {
    DRACE_ENTER_EXCLUDE();
    // This is racy, but we whitelist it
    int val = *v;
    ++val;
    wait();
    *v = val;
    DRACE_LEAVE_EXCLUDE();
  }
}

/**
 * Correctly increment a value by using a spinlock
 * for mutual exclusion. This behaves in user-mode
 * and hence cannot be detected by DRace without annotation.
 */
void spinlock(int* v) {
  static ipc::spinlock mx;
  DRACE_EXCLUDE_STRUCT(mx);

  for (int i = 0; i < NUM_INCREMENTS; ++i) {
    // we expect races on the spinlock itself,
    // as this is not annotated. Simply exclude
    DRACE_ENTER_EXCLUDE();
    mx.lock();
    DRACE_LEAVE_EXCLUDE();
    DRACE_HAPPENS_AFTER(&mx);
    int val = *v;
    ++val;
    wait();
    *v = val;
    DRACE_HAPPENS_BEFORE(&mx);
    DRACE_ENTER_EXCLUDE();
    mx.unlock();
    DRACE_LEAVE_EXCLUDE();
  }
}

int main() {
  int vara = 0;
  int varb = 0;

  std::vector<std::thread> threads;

  threads.emplace_back(&inc, &vara);
  threads.emplace_back(&inc, &vara);

  threads.emplace_back(&spinlock, &varb);
  threads.emplace_back(&spinlock, &varb);

  for (auto& t : threads) {
    t.join();
  }

  std::cout << "data-race happened A: "
            << ((vara == NUM_INCREMENTS * 2) ? "NO" : "YES") << std::endl;
  std::cout << "data-race happened B: "
            << ((varb == NUM_INCREMENTS * 2) ? "NO" : "YES") << std::endl;
  return vara + varb;
}
