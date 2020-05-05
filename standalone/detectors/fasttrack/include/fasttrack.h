#ifndef FASTTRACK_H
#define FASTTRACK_H
#pragma once

/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <detector/Detector.h>
#include <ipc/spinlock.h>
#include <chrono>  //for seeding random generator
#include <iomanip>
#include <iostream>
#include <mutex>   // for lock_guard
#include <random>  // for remove_random_memory_addresses()
#include "parallel_hashmap/phmap.h"
#include "threadstate.h"
#include "varstate.h"
#include "xvector.h"

#define REGARD_ALLOCS true
#define MAKE_OUTPUT false
#define POOL_ALLOC false

///\todo implement a pool allocator;
// Implemented; There exists also a thread-safe version
#if POOL_ALLOC
#include "PoolAllocator.h"
#endif

namespace drace {
namespace detector {

/**
 * \class Fasttrack
 * \brief Main class that implements the FastTrack2 algorithm using the
 * interface provided by the Detector
 * \param LockT a lock used for providing internal mutual exclusion.
 */
template <class LockT>
class Fasttrack : public Detector {
 public:
  typedef size_t tid_ft;
  // make some shared pointers a bit more handy
  typedef std::shared_ptr<ThreadState> ts_ptr;

#if POOL_ALLOC
  template <class X>
  using c_alloc = Allocator<X, 524288>;
#else
  template <class X>
  using c_alloc = std::allocator<X>;
#endif

  explicit Fasttrack() = default;

 private:
  /// holds the callback address to report a race to the drace-main
  Callback clb;
  void* clb_context;

  /// central lock, used for accesses to global tables except vars (order: 1)
  LockT g_lock;

  /// switch logging of read/write operations
  bool log_flag = true;
  bool final_output = (true && log_flag);

  /**
   * \note shared_vcs maps an address to the status of read_shared. we use node,
   * because while an address has a pointer, another one may add to the HashMap,
   * as the locks are on address => we might invalidate our pointers from
   * another adress
   */
  static constexpr uint64_t shared_vcs_number_of_sub_maps = 0x4ull;
  template <class K, class V>
  using phmap_parallel_node_hash_map = phmap::parallel_node_hash_map<
      K, V, phmap::container_internal::hash_default_hash<K>,
      phmap::container_internal::hash_default_eq<K>,
      std::allocator<std::pair<const K, V>>, shared_vcs_number_of_sub_maps,
      ipc::spinlock>;
  phmap_parallel_node_hash_map<std::size_t, xvector<VectorClock<>::VC_EPOCH>>
      shared_vcs;

  /**
   * \note we have to use a node hash map here, as we access it from multiple
   * threads => the HashMap might grow in the meantime => we would
   * invalidate our pointers unless we use a node map
   */
  static constexpr uint64_t vars_number_of_sub_maps = 0x5ull;
  template <class K, class V>
  using phmap_parallel_node_hash_map_no_lock = phmap::parallel_node_hash_map<
      K, V, phmap::container_internal::hash_default_hash<K>,
      phmap::container_internal::hash_default_eq<K>,
      std::allocator<std::pair<const K, V>>,
      vars_number_of_sub_maps,  // number of SubMaps; change here affects
                                // dropSubMap removal policy
      ipc::spinlock>;           // phmap::NullMutex
  phmap_parallel_node_hash_map_no_lock<size_t, VarState> vars;

  /// these maps hold the various state objects together with the identifiers
  phmap::parallel_flat_hash_map<size_t, size_t> allocs;

  /**
   * \note number of locks, threads is expected to be < 1000, hence use one map
   * (without submaps)
   */
  phmap::flat_hash_map<void*, VectorClock<>> locks;
  phmap::flat_hash_map<tid_ft, ts_ptr> threads;
  phmap::parallel_flat_hash_map<void*, VectorClock<>> happens_states;

  /// pool of spinlocks used to synchronize threads when accessing vars HashMap
  /// based on the addr that they need from the Map
  static constexpr uint64_t num_locks = 1024ull;
  static constexpr uint64_t addr_mask = num_locks - 1;
  std::array<ipc::spinlock, num_locks> spinlocks;
  constexpr std::size_t hashOf(std::size_t addr) const {
    return (addr >> 4) & addr_mask;
  }

  /// used in remove_retired_memory_addresses to skip the run through the
  /// HashMap unless the last_min_th_clock modified from the last check
  VectorClock<>::Clock last_min_th_clock = -1;

 public:
  bool init(int argc, const char** argv, Callback rc_clb, void* context) final {
    parse_args(argc, argv);
    clb = rc_clb;  // init callback
    clb_context = context;
    return true;
  }

  void read(tls_t tls, void* pc, void* addr, size_t size) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->set_read_write((size_t)addr, reinterpret_cast<size_t>(pc));
    {  // lock on the address
      std::lock_guard<ipc::spinlock> lg(spinlocks[hashOf((size_t)addr)]);

      // finds the VarState instance of a specific addr or creates it
      auto it = vars.find((size_t)addr);
      if (it == vars.end()) {
#if MAKE_OUTPUT
        std::cout << "variable is read before written" << std::endl;  // warning
#endif
        it = create_var((size_t)addr);
      }
      read(thr, &(it->second), (size_t)addr, size);
    }
  }

  void write(tls_t tls, void* pc, void* addr, size_t size) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->set_read_write((size_t)addr, reinterpret_cast<size_t>(pc));

    {  // lock on the address
      std::lock_guard<ipc::spinlock> lg(spinlocks[hashOf((size_t)addr)]);

      // finds the VarState instance of a specific addr or creates it
      auto it = vars.find((size_t)addr);
      if (it == vars.end()) {
        it = create_var((size_t)addr);
      }

      write(thr, &(it->second), (size_t)addr, size);
    }
  }

  void func_enter(tls_t tls, void* pc) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->get_stack_depot().insert_function_element(
        reinterpret_cast<size_t>(pc));
  }

  void func_exit(tls_t tls) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->get_stack_depot().remove_function_element();
  }

  void fork(tid_t parent, tid_t child, tls_t* tls) final {
    ThreadState* thr;
    std::lock_guard<LockT> exLockT(g_lock);
    auto thrit = threads.find(parent);
    if (thrit != threads.end()) {
      {
        thrit->second->inc_vc();  // inc vector clock for creation of new thread
      }
      thr = create_thread(child, thrit->second);
    } else {
      thr = create_thread(child);
    }
    *tls = reinterpret_cast<tls_t>(thr);
  }

  void join(tid_t parent, tid_t child) final {
    std::lock_guard<LockT> exLockT(g_lock);
    auto del_thread_it = threads.find(child);
    auto parent_it = threads.find(parent);

    // TODO: we should never see invalid ids here.
    // However, after an application fault like in the howto,
    // at least one thread is joined multiple times
    if (del_thread_it == threads.end() || parent_it == threads.end()) {
#if MAKE_OUTPUT
      std::cerr << "invalid thread IDs in join (" << parent << "," << child
                << ")" << std::endl;
#endif
      return;
    }

    ts_ptr del_thread = del_thread_it->second;
    VectorClock<>::ThreadNum del_thread_th_num =
        del_thread_it->second->get_th_num();
    ts_ptr par_thread = parent_it->second;

    par_thread->update(*del_thread);

    threads.erase(del_thread_it);  // no longer do the search
    cleanup(del_thread_th_num);
  }

  // sync thread vc to lock vc
  void acquire(tls_t tls, void* mutex, int recursive, bool write) final {
    if (recursive >
        1) {   // lock haven't been updated by another thread (by definition)
      return;  // therefore no action needed here as only the invoking
               // thread may have updated the lock
    }

    std::lock_guard<LockT> exLockT(g_lock);
    auto it = locks.find(mutex);
    if (it == locks.end()) {
      it = createLock(mutex);
    }
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    (thr)->update(it->second);
  }

  void release(tls_t tls, void* mutex, bool write) final {
    std::lock_guard<LockT> exLockT(g_lock);
    auto it = locks.find(mutex);
    if (it == locks.end()) {
#if MAKE_OUTPUT
      std::cerr << "lock is released but was never acquired by any thread"
                << std::endl;
#endif
      createLock(mutex);
      return;  // as lock is empty (was never acquired), we can return here
    }

    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->inc_vc();

    // increase vector clock and propagate to lock
    it->second.update(thr);  // ThreadState* is VectorClock* (inheritance)
  }

  // identifier = TID of another thread, which happens before our thread
  void happens_before(tls_t tls, void* identifier) final {
    std::lock_guard<LockT> exLockT(g_lock);
    auto it = happens_states.find(identifier);
    if (it == happens_states.end()) {
      it = create_happens(identifier);
    }

    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);

    thr->inc_vc();  // increment clock of thread and update happens state
    it->second.update(thr->get_th_num(), thr->return_own_id());
    // calls update(ThreadNum th_num, VC_EPOCH id)
  }

  void happens_after(tls_t tls, void* identifier) final {
    {
      std::lock_guard<LockT> exLockT(g_lock);
      auto it = happens_states.find(identifier);
      if (it == happens_states.end()) {
        create_happens(identifier);
        return;  // create -> no happens_before can be synced
      } else {
        ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
        // update vector clock of thread with happened before clocks
        thr->update(it->second);  // calls update(VectorClock* other)
      }
    }
  }

  void allocate(tls_t tls, void* pc, void* addr, size_t size) final {
#if REGARD_ALLOCS
    size_t address = reinterpret_cast<size_t>(addr);
    std::lock_guard<LockT> exLockT(g_lock);
    if (allocs.find(address) == allocs.end()) {
      create_alloc(address, size);
    }
#endif
  }

  // when we deallocate a variable we erase it from the VarState
  void deallocate(tls_t tls, void* addr) final {
#if REGARD_ALLOCS
    size_t address = reinterpret_cast<size_t>(addr);
    size_t end_addr;

    std::lock_guard<LockT> exLockT(g_lock);
    auto it = allocs.find(address);
    if (it != allocs.end()) {
      end_addr = address + it->second;
    } else {
      end_addr = 0;
    }

    while (address < end_addr) {  // we deallocate each address
      /**
       * \note Let's say we have a big vector that gets allocated. we track the
       * allocation, but we don't create the Var state objects for the
       * adresses. We create them 1 by 1 on each access. But when we deallocate
       * we clear all of them
       */

      if (vars.find(address) != vars.end()) {
        vars.erase(address);
        address++;
      } else {
        address++;
      }
    }
    allocs.erase(address);
#endif
  }

  void detach(tls_t tls, tid_t thread_id) final {
    /// \todo This is currently not supported
    return;
  }

  void finish(tls_t tls, tid_t thread_id) final {
    std::lock_guard<LockT> exLockT(g_lock);
    /// just delete thread from list, no backward sync needed
    VectorClock<>::ThreadNum th_num;
    auto thrit = threads.find(thread_id);
    th_num = thrit->second->get_th_num();
    threads.erase(thrit);
    cleanup(th_num);
  }

  void finalize() final {
    std::lock_guard<LockT> lg1(g_lock);
    vars.clear();
    locks.clear();
    happens_states.clear();
    allocs.clear();
    threads.clear();
    shared_vcs.clear();
    VectorClock<>::thread_ids.clear();

    if (final_output) process_log_output();
    fflush(stdout);
    std::cout << std::flush;
  }

  /**
   * \note allocation of shadow memory is not required in this
   *       detector. If a standalone version of Fasttrack is used
   *       this function can be ignored
   */
  void map_shadow(void* startaddr, size_t size_in_bytes) final { return; }

  const char* name() final { return "FASTTRACK"; }

  const char* version() final { return "0.0.1"; }

 private:
  /**
   * \brief report a data-race back to DRace
   * \note the function itself must not use locks
   * \note Invariant: this function requires a lock the following global tables:
   *                  threads
   */
  void report_race(uint32_t thr1, uint32_t thr2, bool wr1, bool wr2,
                   const VarState& var, size_t address,
                   std::size_t size) const {
    auto it = threads.find(thr1);
    auto it2 = threads.find(thr2);
    auto it_end = threads.end();

    if (it == it_end ||
        it2 == it_end) {  // if thread_id is, because of finishing, not in
      return;             // stacktraces anymore, return
    }
    std::deque<size_t> stack1(
        std::move(it->second->return_stack_trace(address)));
    std::deque<size_t> stack2(
        std::move(it2->second->return_stack_trace(address)));

    while (stack1.size() > Detector::max_stack_size) {
      stack1.pop_front();
    }
    while (stack2.size() > Detector::max_stack_size) {
      stack2.pop_front();
    }

    Detector::AccessEntry access1;
    access1.thread_id = thr1;
    access1.write = wr1;
    access1.accessed_memory = address;
    access1.access_size = size;
    access1.access_type = 0;
    access1.heap_block_begin = 0;
    access1.heap_block_size = 0;
    access1.onheap = false;
    access1.stack_size = stack1.size();
    std::copy(stack1.begin(), stack1.end(), access1.stack_trace.begin());

    Detector::AccessEntry access2;
    access2.thread_id = thr2;
    access2.write = wr2;
    access2.accessed_memory = address;
    access2.access_size = size;
    access2.access_type = 0;
    access2.heap_block_begin = 0;
    access2.heap_block_size = 0;
    access2.onheap = false;
    access2.stack_size = stack2.size();
    std::copy(stack2.begin(), stack2.end(), access2.stack_trace.begin());

    Detector::Race race;
    race.first = access1;
    race.second = access2;

    clb(&race, clb_context);
  }

  /**
   * \brief Wrapper for report_race to use const qualifier on wrapped function
   */
  void report_race_locked(uint32_t thr1, uint32_t thr2, bool wr1, bool wr2,
                          const VarState& var, std::size_t addr,
                          std::size_t size) {
    std::lock_guard<LockT> lg(g_lock);
    report_race(thr1, thr2, wr1, wr2, var, addr, size);
  }

  /**
   * \brief takes care of a read access
   * \note works only on calling-thread and var object, not on any list
   */
  void read(ThreadState* t, VarState* v, std::size_t addr, std::size_t size) {
    if (t->return_own_id() ==
        v->get_read_epoch()) {  // read same epoch, same thread;
      if (log_flag) {
        log_count.read_ex_same_epoch++;
      }
      return;
    }

    VectorClock<>::VC_EPOCH id = t->return_own_id();  // id means epoch
    VectorClock<>::ThreadNum th_num = t->get_th_num();

    xvector<VectorClock<>::VC_EPOCH>* shared_vc;
    {
      auto it = shared_vcs.find(addr);
      /// returns the vector when read is shared
      if (it != shared_vcs.end()) {  // if it exists get a pointer to the vector
        shared_vc = &(it->second);   // vector copy would be too expensive
      } else {  // if I find it is read_shared. else it is not
        shared_vc = nullptr;
      }
    }

    if (shared_vc != nullptr && v->get_vc_by_th_num(th_num, shared_vc) ==
                                    id) {  // read shared same epoch
      if (log_flag) {
        log_count.read_sh_same_epoch++;
      }
      return;
    }

    if (v->is_wr_race(t)) {  // write-read race
      if (log_flag) {
        log_count.wr_race++;
      }
      report_race_locked(v->get_write_thread_id(), t->get_tid(), true, false,
                         *v, addr, size);
    }

    // update vc
    if (shared_vc == nullptr) {  // if it's not read shared
      if (v->get_read_epoch() == VarState::VAR_NOT_INIT ||
          (v->get_read_thread_num() ==
           th_num)) {  // read exclusive => read of same thread but newer epoch
        update_var_state(false, id, v, addr, shared_vc);
        if (log_flag) {
          log_count.read_exclusive++;
        }
      } else {  //=> means that (v->get_read_thread_id() != tid) && or variable
                // hasn't
                // been read yet read gets shared
        set_read_shared_state(id, v, addr);
        if (log_flag) {
          log_count.read_share++;
        }
      }
    } else {  // read shared
      update_var_state(false, id, v, addr, shared_vc);
      if (log_flag) {
        log_count.read_shared++;
      }
    }
  }

  /// updates the var state because of a new read or write access
  void update_var_state(bool is_write, VectorClock<>::VC_EPOCH id, VarState* v,
                        std::size_t addr,
                        xvector<VectorClock<>::VC_EPOCH>* shared_vc) {
    if (is_write) {
      if (shared_vc != nullptr) {
        shared_vcs.erase(addr);
        v->set_read_epoch(VarState::VAR_NOT_INIT);
      }
      v->set_write_epoch(id);
      return;
    }

    // read, but not in shared mode
    if (shared_vc == nullptr) {
      v->set_read_epoch(id);
      return;
    }

    // read in shared mode
    auto it =
        VarState::find_in_vec(VectorClock<>::make_thread_num(id), shared_vc);
    if (it != shared_vc->end()) {
      (*it) = id;  // just update the value
    } else {
      // if id was not found, we add it to the read_shared vector
      shared_vc->push_back(id);
    }
  }

  /// sets read state of the address to shared
  void set_read_shared_state(VectorClock<>::VC_EPOCH id, VarState* v,
                             std::size_t addr) {
    xvector<VectorClock<>::VC_EPOCH>* tmp;
    tmp = &(shared_vcs.emplace(addr, xvector<VectorClock<>::VC_EPOCH>(2))
                .first->second);
    tmp->operator[](0) = (v->get_read_epoch());
    tmp->operator[](1) = (id);
    v->set_read_epoch(VarState::READ_SHARED);
  }

  /**
   * \brief takes care of a write access
   * \note works only on calling-thread and var object, not on any list
   */
  void write(ThreadState* t, VarState* v, std::size_t addr, std::size_t size) {
    if (t->return_own_id() == v->get_write_epoch()) {  // write same epoch
      if (log_flag) {
        log_count.write_same_epoch++;
      }
      return;
    }

    VectorClock<>::ThreadNum th_num = t->get_th_num();
    VectorClock<>::TID tid = t->get_tid();

    // tids are different and write epoch greater or
    // equal than known epoch of other thread
    if (v->is_ww_race(t)) {  // write-write race
      if (log_flag) {
        log_count.ww_race++;
      }
      report_race_locked(v->get_write_thread_id(), tid, true, true, *v, addr,
                         size);
    }

    xvector<VectorClock<>::VC_EPOCH>* shared_vc;
    {
      auto it = shared_vcs.find(addr);
      if (it != shared_vcs.end()) {
        shared_vc = &(it->second);  // vector copy would be too expensive
      } else {  // if I find it is read_shared. else it is not
        shared_vc = nullptr;
      }
    }

    if (shared_vc == nullptr) {
      if (log_flag) {
        log_count.write_exclusive++;
      }
      if (v->is_rw_ex_race(t)) {  // read-write race
        if (log_flag) {
          log_count.rw_ex_race++;
        }
        report_race_locked(v->get_read_thread_id(), tid, false, true, *v, addr,
                           size);
      }
    } else {  // come here in read shared case
              // TODO: search here for the shared_vc
      if (log_flag) {
        log_count.write_shared++;
      }
      VectorClock<>::ThreadNum act_th_num = v->is_rw_sh_race(t, shared_vc);
      if (act_th_num != 0)  // we return 0 & we start counting ThreadNum from 1
      {                     // read shared read-write race
        if (log_flag) {
          log_count.rw_sh_race++;
        }
        report_race_locked(VectorClock<>::make_tid_from_th_num(act_th_num), tid,
                           false, true, *v, addr, size);
      }
    }
    update_var_state(true, t->return_own_id(), v, addr, shared_vc);
  }

  /**
   * \brief creates a new variable object (is called, when var is read or
   * written for the first time) \note Invariant: vars table is locked
   */
  inline auto create_var(size_t addr) {
    return vars.emplace(addr, VarState()).first;
  }

  /// creates a new lock object (is called when a lock is acquired or released
  /// for the first time)
  inline auto createLock(void* mutex) {
    return locks
        .emplace(std::piecewise_construct, std::forward_as_tuple(mutex),
                 std::forward_as_tuple())
        .first;
  }

  /// creates a new thread object (is called when fork() called)
  inline ThreadState* create_thread(VectorClock<>::TID tid,
                                    ts_ptr parent = nullptr) {
    ts_ptr new_thread;

    if (nullptr == parent) {
      // This does not work be
      // new_thread = threads.emplace(tid, std::make_shared<ThreadState>(tid))
      //                  .first->second;
      new_thread =
          threads
              .emplace(tid, std::shared_ptr<ThreadState>(new ThreadState(tid)))
              .first->second;
    } else {
      new_thread = threads
                       .emplace(tid, std::shared_ptr<ThreadState>(
                                         new ThreadState(tid, parent)))
                       .first->second;
    }
    return new_thread.get();
  }

  /// creates a happens_before object
  inline auto create_happens(void* identifier) {
    return happens_states
        .emplace(std::piecewise_construct, std::forward_as_tuple(identifier),
                 std::forward_as_tuple())
        .first;
  }

  /// creates an allocation object
  inline void create_alloc(size_t addr, size_t size) {
    allocs.emplace(addr, size);
  }

  void parse_args(int argc, const char** argv) {
    int processed = 1;
    while (processed < argc) {
      if (strcmp(argv[processed], "--stats") == 0) {
        log_flag = true;
      }
      ++processed;
    }
  }

  /**
   * \brief deletes all data which is related to the tid
   * is called when a thread finishes (either from \ref join() or from \ref
   * finish()) \note Not Threadsafe
   */
  void cleanup(VectorClock<>::ThreadNum th_num) {
    {
      for (auto it = locks.begin(); it != locks.end(); ++it) {
        it->second.delete_vc(th_num);
      }

      for (auto it = threads.begin(); it != threads.end(); ++it) {
        it->second->delete_vc(th_num);
      }

      for (auto it = happens_states.begin(); it != happens_states.end(); ++it) {
        it->second.delete_vc(th_num);
      }
    }
  }

 public:
  // protected:  // the log counters are public for testing
  /**
   * \note the things from here are used for both reporting back to the user
   * statistics and for testing, as one can easily check that log counters
   * correspond to check the correctness of the algorithm
   */
  /// statistics
  struct log_counters {
    uint32_t read_ex_same_epoch = 0;
    uint32_t read_sh_same_epoch = 0;
    uint32_t read_shared = 0;
    uint32_t read_exclusive = 0;
    uint32_t read_share = 0;
    uint32_t write_same_epoch = 0;
    uint32_t write_exclusive = 0;
    uint32_t write_shared = 0;
    uint32_t wr_race = 0;
    uint32_t rw_sh_race = 0;
    uint32_t ww_race = 0;
    uint32_t rw_ex_race = 0;
  } log_count;

  /// print statistics about rule-hits
  void process_log_output() const {
    double read_actions, write_actions;
    // percentages
    double rd, wr, r_ex_se, r_sh_se, r_ex, r_share, r_shared, w_se, w_ex, w_sh;

    read_actions = log_count.read_ex_same_epoch + log_count.read_sh_same_epoch +
                   log_count.read_exclusive + log_count.read_share +
                   log_count.read_shared;
    write_actions = log_count.write_same_epoch + log_count.write_exclusive +
                    log_count.write_shared;

    rd = (read_actions / (read_actions + write_actions)) * 100;
    wr = 100 - rd;
    r_ex_se = (log_count.read_ex_same_epoch / read_actions) * 100;
    r_sh_se = (log_count.read_sh_same_epoch / read_actions) * 100;
    r_ex = (log_count.read_exclusive / read_actions) * 100;
    r_share = (log_count.read_share / read_actions) * 100;
    r_shared = (log_count.read_shared / read_actions) * 100;
    w_se = (log_count.write_same_epoch / write_actions) * 100;
    w_ex = (log_count.write_exclusive / write_actions) * 100;
    w_sh = (log_count.write_shared / write_actions) * 100;

    std::cout << "FASTTRACK_STATISTICS: All values are percentages!"
              << std::endl;
    std::cout << std::fixed << std::setprecision(2) << "Read Actions: " << rd
              << std::endl;
    std::cout << "Of which: " << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << "Read exclusive same epoch: " << r_ex_se << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << "Read shared same epoch: " << r_sh_se << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << "Read exclusive: " << r_ex << std::endl;
    std::cout << std::fixed << std::setprecision(2) << "Read share: " << r_share
              << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << "Read shared: " << r_shared << std::endl;
    std::cout << std::endl;
    std::cout << std::fixed << std::setprecision(2) << "Write Actions: " << wr
              << std::endl;
    std::cout << "Of which: " << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << "Write same epoch: " << w_se << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << "Write exclusive: " << w_ex << std::endl;
    std::cout << std::fixed << std::setprecision(2) << "Write shared: " << w_sh
              << std::endl;
    std::cout << "====----------- FASTTRACK_DETAILS: Values are absolute! "
                 "-----------===="
              << std::endl;
  }

  // helper funtion for a unit_test
  void clear_var_state(std::size_t addr) {
    auto it = vars.find((size_t)(addr));
    if (it != vars.end()) {
      vars.erase(it);
    }
  }
};
}  // namespace detector
}  // namespace drace
#endif  // !FASTTRACK_H