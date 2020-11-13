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

#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <map>
#include <mutex>  // for lock_guard
#include <unordered_map>
#include <vector>

#include <detector/Detector.h>

#include <ipc/spinlock.h>
#include <tsan-if.h>

namespace drace {
namespace detector {
/// tsan detector based on LLVM ThreadSanitizer
class Tsan : public Detector {
  struct tsan_params_t {
    bool heap_only{false};
  } params;

  struct ThreadState {
    Detector::tls_t tsan;
    bool active;
  };

  /// reserved area at begin of shadow memory addr range;
  static constexpr uint64_t MEMSTART = 0xc0'0000'0000ull;

  /* Cannot use std::mutex here, hence use spinlock */
  ipc::spinlock mxspin;
  std::unordered_map<uint64_t, size_t> allocations;
  std::unordered_map<Detector::tid_t, ThreadState> thread_states;

  static void reportRaceCallBack(__tsan_race_info* raceInfo,
                                 void* add_race_clb) {
    // Fixes erronous thread exit handling by ignoring races where at least one
    // tid is 0
    if (!raceInfo->access1->user_id || !raceInfo->access2->user_id) return;

    Race race;

    // TODO: Currently we assume that there is at most one callback at a time
    __tsan_race_info_access* race_info_ac;
    for (int i = 0; i != 2; ++i) {
      AccessEntry access;
      if (i == 0) {
        race_info_ac = raceInfo->access1;
      } else {
        race_info_ac = raceInfo->access2;
      }

      access.thread_id = (unsigned)race_info_ac->user_id;
      access.write = race_info_ac->write;

      // only report last 32 bits until full 64 bit support is implemented in
      // i#11
      uint64_t addr = (uint64_t)(race_info_ac->accessed_memory) & 0xFFFFFFFF;
      access.accessed_memory = addr;
      access.access_size = race_info_ac->size;
      access.access_type = race_info_ac->type;

      size_t ssize = std::min(race_info_ac->stack_trace_size, max_stack_size);
      if (ssize == 0) {
        // TODO: this should not happen
        return;
      }
      memcpy(access.stack_trace.data(), race_info_ac->stack_trace,
             ssize * sizeof(uint64_t));
      access.stack_size = ssize;

      if (i == 0) {
        race.first = access;
      } else {
        race.second = access;
      }
    }
    ((void (*)(const Race*))add_race_clb)(&race);
  }

  /*
   * \brief split address at 32-bit boundary (zero above)
   *
   * TSAN internally uses a shadow memory block to track all
   * accesses. Here, we use a fixed 32-bit block and map
   * our accesses to this block by just considering the lower
   * half. This leads to false positives, if the mapping is not
   * unique. However even with extensive tests on Windows, we
   * never discovered such a situation.
   *
   * \todo Find a solution to reliably detect all stack and heap
   *       regions to finally work with full 64-bit addresses
   */
  static constexpr uint64_t translate(uint64_t addr) {
    return (addr & 0x00000000FFFFFFFF) + MEMSTART;
  }

  /** Fast trivial hash function using a prime. We expect tids in [1,10^5] */
  static constexpr uint64_t get_event_id(tid_t parent, tid_t child) {
    return translate(parent * 65521 + child);
  }
  static constexpr uint64_t get_event_id(tls_t parent, tid_t child) {
    // not ideal as symmetric
    return translate((uint64_t)(parent) + child);
  }
  static constexpr uint64_t get_event_id(tls_t parent, tls_t child) {
    // not ideal as symmetric
    return translate((uint64_t)(parent) ^ (uint64_t)(child));
  }

  void parse_args(int argc, const char** argv) {
    int processed = 1;
    while (processed < argc) {
      if (strncmp(argv[processed], "--heap-only", 16) == 0) {
        params.heap_only = true;
        ++processed;
      } else {
        ++processed;
      }
    }
  }
  void print_config() {
    std::cout << "> Detector Configuration:\n"
              << "> heap-only: " << (params.heap_only ? "ON" : "OFF")
              << std::endl
              << "> version:   " << version() << std::endl;
  }

 public:
  virtual bool init(int argc, const char** argv, Callback rc_clb) {
    parse_args(argc, argv);
    print_config();

    thread_states.reserve(128);

    __tsan_init_simple(reportRaceCallBack, (void*)rc_clb);

    // we fake the go-mapping here, until we have implemented
    // the allocation interceptors in i#11

    /* Go on windows
    0000 0000 1000 - 0000 1000 0000: executable
    0000 1000 0000 - 00f8 0000 0000: -
    00c0 0000 0000 - 00e0 0000 0000: heap
    00e0 0000 0000 - 0100 0000 0000: -
    0100 0000 0000 - 0500 0000 0000: shadow
    0500 0000 0000 - 0560 0000 0000: -
    0560 0000 0000 - 0760 0000 0000: traces
    0760 0000 0000 - 07d0 0000 0000: metainfo (memory blocks and sync objects)
    07d0 0000 0000 - 8000 0000 0000: -
    */

    // map a 32 bit (4GB) heap
    __tsan_map_shadow((void*)MEMSTART, (size_t)(0xFFFFFFFF));
    return true;
  }

  virtual void finalize() {
    for (auto& t : thread_states) {
      if (t.second.active) finish(t.second.tsan, t.first);
    }
    __tsan_fini();
    // do not perform IO here, as it crashes / interfers with dotnet
    // std::cout << "> ----- SUMMARY -----" << std::endl;
    // std::cout << "> Found " << races.load(std::memory_order_relaxed) << "
    // possible data-races" << std::endl; std::cout << "> Detector missed " <<
    // misses.load() << " possible heap refs" << std::endl;
  }

  virtual const char* name() { return "TSAN"; }

  virtual const char* version() { return "0.2.0"; }

  virtual void map_shadow(void* startaddr, size_t size_in_bytes) {
    __tsan_map_shadow(startaddr, static_cast<unsigned long>(size_in_bytes));
  }

  virtual void func_enter(tls_t tls, void* pc) { __tsan_func_enter(tls, pc); }

  virtual void func_exit(tls_t tls) { __tsan_func_exit(tls); }

  virtual void acquire(tls_t tls, void* mutex, int rec, bool write) {
    uint64_t addr_32 = translate((uint64_t)mutex);
    // if the mutex is a handle, the ID might be too small
    if (addr_32 < MEMSTART) addr_32 += MEMSTART;

    assert(nullptr != tls);

    __tsan_mutex_after_lock(tls, (void*)addr_32, (void*)write);
  }

  virtual void release(tls_t tls, void* mutex, bool write) {
    uint64_t addr_32 = translate((uint64_t)mutex);
    // if the mutex is a handle, the ID might be too small
    if (addr_32 < MEMSTART) addr_32 += MEMSTART;

    assert(nullptr != tls);

    __tsan_mutex_before_unlock(tls, (void*)addr_32, (void*)write);
  }

  virtual void happens_before(tls_t tls, void* identifier) {
    uint64_t addr_32 = translate((uint64_t)identifier);
    __tsan_happens_before(tls, (void*)addr_32);
  }

  virtual void happens_after(tls_t tls, void* identifier) {
    uint64_t addr_32 = translate((uint64_t)identifier);
    __tsan_happens_after(tls, (void*)addr_32);
  }

  virtual void read(tls_t tls, void* pc, void* addr, size_t size) {
    uint64_t addr_32 = translate((uint64_t)addr);
    __tsan_read(tls, (void*)addr_32, (void*)pc);
  }

  virtual void write(tls_t tls, void* pc, void* addr, size_t size) {
    uint64_t addr_32 = translate((uint64_t)addr);
    __tsan_write(tls, (void*)addr_32, (void*)pc);
  }

  /**
   * \todo The current handling of allocations in tsan port is buggy.
   *  for now we do not rely on this, hence just short-circuit
   */
  virtual void allocate(tls_t tls, void* pc, void* addr, size_t size) {
    return;
#if 0
                uint64_t addr_32 = translate((uint64_t)addr);

                __tsan_malloc(tls, pc, (void*)addr_32, size);

                {
                    std::lock_guard<ipc::spinlock> lg(mxspin);
                    allocations.emplace(addr_32, size);
                }
#endif
  }

  /**
   * \todo The current handling of allocations in tsan port is buggy.
   *  for now we do not rely on this, hence just short-circuit
   */
  virtual void deallocate(tls_t tls, void* addr) {
    return;
#if 0
                uint64_t addr_32 = translate((uint64_t)addr);

                mxspin.lock();
                // ocasionally free is called more often than allocate, hence guard
                if (allocations.count(addr_32) == 1) {
                    size_t size = allocations[addr_32];
                    allocations.erase(addr_32);
                    __tsan_free((void*)addr_32, size);
                }
                mxspin.unlock();
#endif
  }

  virtual void fork(tid_t parent, tid_t child, tls_t* tls) {
    const uint64_t event_id = get_event_id(parent, child);
    *tls = __tsan_create_thread(child);

    std::lock_guard<ipc::spinlock> lg(mxspin);

    for (const auto& t : thread_states) {
      if (t.second.active) {
        assert(nullptr != t.second.tsan);
        __tsan_happens_before(t.second.tsan, (void*)(event_id));
      }
    }
    __tsan_happens_after(*tls, (void*)(event_id));
    thread_states[child] = ThreadState{*tls, true};
  }

  virtual void join(tid_t parent, tid_t child) {
    const uint64_t event_id = get_event_id(parent, child);

    std::lock_guard<ipc::spinlock> lg(mxspin);
    thread_states[child].active = false;

    const auto thr = thread_states[child].tsan;
    assert(nullptr != thr);
    __tsan_happens_before(thr, (void*)(event_id));
    for (const auto& t : thread_states) {
      if (t.second.active) {
        assert(nullptr != t.second.tsan);
        __tsan_happens_after(t.second.tsan, (void*)(event_id));
      }
    }
    // we cannot use __tsan_ThreadJoin here, as local tid is not tracked
    // hence use thread finish and draw edge between exited thread
    // and all other threads
    __tsan_ThreadFinish(thread_states[child].tsan);
  }

  virtual void detach(tls_t tls, tid_t thread_id) {
    /// \todo This is currently not supported
    return;
  }

  virtual void finish(tls_t tls, tid_t thread_id) {
    if (tls != nullptr) {
      __tsan_go_end(tls);
    }
    std::lock_guard<ipc::spinlock> lg(mxspin);
    thread_states[thread_id].active = false;
  }
};
}  // namespace detector
}  // namespace drace

extern "C" __declspec(dllexport) Detector* CreateDetector() {
  return new drace::detector::Tsan();
}
