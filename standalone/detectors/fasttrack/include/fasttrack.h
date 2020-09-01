#ifndef FASTTRACK_H
#define FASTTRACK_H
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

#include <detector/Detector.h>
#include <ipc/spinlock.h>
#include <parallel_hashmap/phmap_utils.h>  // minimal header providing phmap::HashState()
#include <chrono>   //for profiling + seeding random generator
#include <fstream>  //for writing values of hash function to file
#include <iomanip>
#include <iostream>
#include <mutex>   // for lock_guard
#include <random>  // for removeRandomVarState
#include "parallel_hashmap/phmap.h"
#include "stacktrace.h"
#include "threadstate.h"
#include "varstate.h"
#include "xvector.h"

#define REGARD_ALLOCS true
#define MAKE_OUTPUT false
#define POOL_ALLOC false

/*
---------------------------------------------------------------------
Define for easier debugging and profiling for optimization
---------------------------------------------------------------------
*/
#include "prof.h"
#define PROF_INFO false

///\todo implement a pool allocator
#if POOL_ALLOC
#include "util/PoolAllocator.h"
#endif

namespace drace {
namespace detector {

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

 private:
  /// holds the callback address to report a race to the drace-main
  Callback clb;

  /// central lock, used for accesses to global tables except vars (order: 1)
  LockT g_lock;

  /*
---------------------------------------------------------------------
 Variables to be defined via an external framework
---------------------------------------------------------------------
*/
  /// switch logging of read/write operations
  bool log_flag = true;
  bool final_output = (true && log_flag);

#define DELETE_POLICY false
  bool _flag_removeUselessVarStates = (true && DELETE_POLICY);
  bool _flag_removeDropSubMaps = (false && DELETE_POLICY);
  bool _flag_removeRandomVarStates = (true && DELETE_POLICY);
  bool _flag_removeLowestClockVarStates = (false && DELETE_POLICY);
  std::size_t vars_size = 50000;  // TODO: optimal threshold

  /// these maps hold the various state objects together with the identifiers
  phmap::parallel_flat_hash_map<size_t, size_t> allocs;

  // shared_vcs maps addr to the status of read_shared. we use node, because
  // while an address has a pointer, another one may add to the HashMap,
  // as the locks are on address => we might invalidate our pointers from
  // another adress
  template <class K, class V>
  using phmap_parallel_node_hash_map = phmap::parallel_node_hash_map<
      K, V, phmap::container_internal::hash_default_hash<K>,
      phmap::container_internal::hash_default_eq<K>,
      std::allocator<std::pair<const K, V>>, 4, ipc::spinlock>;
  phmap_parallel_node_hash_map<std::size_t, xvector<VectorClock<>::VC_ID>>
      shared_vcs;

  // we have to use a node hash map here, as we access it from multiple
  // threads => the HashMap might grow in the meantime => we would
  // invalidate our pointers unless we use a node map
  template <class K, class V>
  using phmap_parallel_node_hash_map_no_lock = phmap::parallel_node_hash_map<
      K, V, phmap::container_internal::hash_default_hash<K>,
      phmap::container_internal::hash_default_eq<K>,
      std::allocator<std::pair<const K, V>>, 5,
      ipc::spinlock>;  // phmap::NullMutex
  phmap_parallel_node_hash_map_no_lock<size_t, VarState> vars;

  // number of locks, threads is expected to be < 1000, hence use one map
  // (without submaps)
  phmap::flat_hash_map<void*, VectorClock<>> locks;
  phmap::flat_hash_map<tid_ft, ts_ptr> threads;
  phmap::parallel_flat_hash_map<void*, VectorClock<>> happens_states;

  /// pool of spinlocks used to synchronize threads when accessing vars HashMap
  /// based on the addr that they need from the Map
  std::array<ipc::spinlock, 1024> spinlocks;

  // used in removeUselessVarStates to skip the run through the HashMap unless
  // the last_min_th_clock modified from the last check
  VectorClock<>::Clock _last_min_th_clock = -1;

  std::ofstream _hash_file;

 public:
  explicit Fasttrack() = default;

  bool init(int argc, const char** argv, Callback rc_clb) final {
    parse_args(argc, argv);
    clb = rc_clb;  // init callback
    vars.reserve(65535);
    _hash_file.open("key_hash_values.txt");
    return true;
  }

  // helper funtion for a unit_test -> might have its usage later
  void clearVarState(std::size_t addr) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    auto it = vars.find((size_t)(addr));
    if (it != vars.end()) {
      vars.erase(it);
    }
  }

  void read(tls_t tls, void* pc, void* addr, size_t size) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);

    {  // lock on the address
      std::lock_guard<ipc::spinlock> lg(spinlocks[hashOf((std::size_t)addr)]);

      VarState* var;
      {  // finds the VarState instance of a specific addr or creates it
#if DELETE_POLICY
        if (vars.size() >= vars_size) {
          clearVarStates();
        }
#endif
#if PROF_INFO
        PROF_START_BLOCK("find")
#endif
        //{
        //  std::lock_guard<LockT> exLockT(g_lock);
        //  vars.hash_test((size_t)addr, _hash_file);
        //}
        auto it = vars.find((size_t)addr, (size_t)addr);
        if (it == vars.end()) {
#if MAKE_OUTPUT
          std::cout << "variable is read before written"
                    << std::endl;  // warning
#endif
          it = create_var((size_t)(addr));
        }
        var = &(it->second);
#if PROF_INFO
        PROF_END_BLOCK
#endif
      }
      set_read_write(var, thr, pc);
      read(thr, var, (size_t)addr, size);
    }
  }

  void write(tls_t tls, void* pc, void* addr, size_t size) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);

    {  // lock on the address
      std::lock_guard<ipc::spinlock> lg(spinlocks[hashOf((std::size_t)addr)]);

      VarState* var;
      {  // finds the VarState instance of a specific addr or creates it
#if DELETE_POLICY
        if (vars.size() >= vars_size) {
          clearVarStates();
        }
#endif
#if PROF_INFO
        PROF_START_BLOCK("find")
#endif
        auto it = vars.find((size_t)addr, (size_t)addr);
        if (it == vars.end()) {
          it = create_var((size_t)(addr));
        }
        var = &(it->second);
#if PROF_INFO
        PROF_END_BLOCK
#endif
      }
      set_read_write(var, thr, pc);
      write(thr, var, (size_t)addr, size);
    }
  }

  void func_enter(tls_t tls, void* pc) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->get_stackDepot().push_stack_element(reinterpret_cast<size_t>(pc));
  }

  void func_exit(tls_t tls) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
    thr->get_stackDepot().pop_stack_element();
  }

  void fork(tid_t parent, tid_t child, tls_t* tls) final {
    ThreadState* thr;

    std::lock_guard<LockT> exLockT(g_lock);
    auto thrit = threads.find(parent);
    if (thrit != threads.end()) {
      {
        thrit->second->inc_vc();  // inc vector clock for creation of new thread
      }
      thr = create_thread(child, threads[parent]);
    } else {
      thr = create_thread(child);
    }
    *tls = reinterpret_cast<tls_t>(thr);
  }

  void join(tid_t parent, tid_t child) final {
#if PROF_INFO
    PROF_FUNCTION();
#endif
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
    VectorClock<>::Thread_Num del_thread_th_num =
        del_thread_it->second->get_th_num();
    ts_ptr par_thread = parent_it->second;

    par_thread->update(*del_thread);

    removeVarStatesOfThread(del_thread_th_num);

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
    // calls update(Thread_Num th_num, VC_ID id)
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
    end_addr = address + allocs[address];  // allocs[address] = size;
    // TODO: use allocs.find

    // variable is deallocated so varstate objects can be destroyed
    while (address < end_addr) {  // we deallocate each address
      /* Let's say we have a big vector that gets allocated. we track the
      allocation, but we don't create the Var state objects for the
      adresses. We create them 1 by 1 on each access. But when we deallocate
      we clear all of them.*/
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
#if PROF_INFO
    PROF_FUNCTION();
#endif
    std::lock_guard<LockT> exLockT(g_lock);
    /// just delete thread from list, no backward sync needed
    VectorClock<>::Thread_Num th_num;
    auto thrit = threads.find(thread_id);
    th_num = thrit->second->get_th_num();

    threads.erase(thrit);
    cleanup(th_num);
  }

  void finalize() final {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    std::lock_guard<LockT> lg1(g_lock);
    vars.clear();
    locks.clear();
    happens_states.clear();
    allocs.clear();
    threads.clear();
    shared_vcs.clear();
    VectorClock<>::thread_ids.clear();

    if (final_output) process_log_output();
#if PROF_INFO
    ProfTimer::Print();
#endif
    fflush(stdout);
    std::cout << std::flush;
    _hash_file.close();
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
  static constexpr unsigned long long _addr_mask = 0x3FFull;
  constexpr std::size_t hashOf(std::size_t addr) const {
    return (addr >> 4) & _addr_mask;
  }

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
    std::list<size_t> stack1(
        std::move(return_stack_trace(var, it->second.get())));
    std::list<size_t> stack2(
        std::move(return_stack_trace(var, it2->second.get())));

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
    std::copy(stack1.begin(), stack1.end(), access1.stack_trace);

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
    std::copy(stack2.begin(), stack2.end(), access2.stack_trace);

    Detector::Race race;
    race.first = access1;
    race.second = access2;

    ((void (*)(const Detector::Race*))clb)(&race);
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
#if PROF_INFO
    PROF_FUNCTION()
#endif
    if (t->return_own_id() ==
        v->get_read_id()) {  // read same epoch, same thread;
      if (log_flag) {
        log_count.read_ex_same_epoch++;
      }
      return;
    }

    VectorClock<>::VC_ID id = t->return_own_id();  // id means epoch
    VectorClock<>::Thread_Num th_num = t->get_th_num();

    xvector<VectorClock<>::VC_ID>* shared_vc;
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
      report_race_locked(v->get_w_tid(), t->get_tid(), true, false, *v, addr,
                         size);
    }

    // update vc
    if (shared_vc == nullptr) {  // if it's not read shared
      if (v->get_read_id() == VarState::VAR_NOT_INIT ||
          (v->get_r_th_num() ==
           th_num)) {  // read exclusive => read of same thread but newer epoch
        update_VarState(false, id, v, addr, shared_vc);
        if (log_flag) {
          log_count.read_exclusive++;
        }
      } else {  //=> means that (v->get_r_tid() != tid) && or variable hasn't
                // been read yet read gets shared
        set_read_shared(id, v, addr);
        if (log_flag) {
          log_count.read_share++;
        }
      }
    } else {  // read shared
      update_VarState(false, id, v, addr, shared_vc);
      if (log_flag) {
        log_count.read_shared++;
      }
    }
  }

  /// updates the var state because of a new read or write access through a
  /// thread
  void update_VarState(bool is_write, VectorClock<>::VC_ID id, VarState* v,
                       std::size_t addr,
                       xvector<VectorClock<>::VC_ID>* shared_vc) {
#if PROF_INFO
    PROF_FUNCTION()
#endif

    if (is_write) {  // we have to do shared_vcs.erase() here
                     // shared_vcs_it is for sure a phmap::iterator
      if (shared_vc != nullptr) {
        shared_vcs.erase(addr);
      }
      v->r_id = VarState::VAR_NOT_INIT;
      v->w_id = id;
      return;
    }

    // read, but not in shared mode
    if (shared_vc == nullptr) {
      v->r_id = id;
      return;
    }

    // read in shared mode
    auto it = VarState::find_in_vec(VectorClock<>::make_th_num(id), shared_vc);
    if (it != shared_vc->end()) {
      (*it) = id;  // just update the value
    } else {
      // if id was not found, we add it to the read_shared vector
      shared_vc->push_back(id);
    }
  }

  /// sets read state of the address to shared
  void set_read_shared(VectorClock<>::VC_ID id, VarState* v, std::size_t addr) {
    xvector<VectorClock<>::VC_ID>* tmp;
    tmp = &(shared_vcs.emplace(addr, xvector<VectorClock<>::VC_ID>(2))
                .first->second);
    tmp->operator[](0) = (v->r_id);
    tmp->operator[](1) = (id);
    v->r_id = VarState::R_ID_SHARED;
  }

  /**
   * \brief takes care of a write access
   * \note works only on calling-thread and var object, not on any list
   */
  void write(ThreadState* t, VarState* v, std::size_t addr, std::size_t size) {
#if PROF_INFO
    PROF_FUNCTION()
#endif
    if (t->return_own_id() == v->get_write_id()) {  // write same epoch
      if (log_flag) {
        log_count.write_same_epoch++;
      }
      return;
    }

    VectorClock<>::Thread_Num th_num = t->get_th_num();
    VectorClock<>::TID tid = t->get_tid();

    // tids are different and write epoch greater or
    // equal than known epoch of other thread
    if (v->is_ww_race(t)) {  // write-write race
      if (log_flag) {
        log_count.ww_race++;
      }
      report_race_locked(v->get_w_tid(), tid, true, true, *v, addr, size);
    }

    xvector<VectorClock<>::VC_ID>* shared_vc;
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
        report_race_locked(v->get_r_tid(), tid, false, true, *v, addr, size);
      }
    } else {  // come here in read shared case
              // TODO: search here for the shared_vc
      if (log_flag) {
        log_count.write_shared++;
      }
      VectorClock<>::Thread_Num act_th_num = v->is_rw_sh_race(t, shared_vc);
      if (act_th_num != 0)  // we return 0 & we start counting Thread_Num from 1
      {                     // read shared read-write race
        if (log_flag) {
          log_count.rw_sh_race++;
        }
        report_race_locked(VectorClock<>::make_tid_from_th_num(act_th_num), tid,
                           false, true, *v, addr, size);
      }
    }
    update_VarState(true, t->return_own_id(), v, addr, shared_vc);
  }

  /**
   * \brief creates a new variable object (is called, when var is read or
   * written for the first time) \note Invariant: vars table is locked
   */
  inline auto create_var(size_t addr) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (log_flag) {
      log_count.no_allocatedVarStates++;
    }
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
      new_thread = threads.emplace(tid, std::make_shared<ThreadState>(tid))
                       .first->second;
    } else {
      new_thread =
          threads.emplace(tid, std::make_shared<ThreadState>(tid, parent))
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
      if (strcmp(argv[processed], "--size") == 0) {
        processed++;
        vars_size = std::stoi(argv[processed]);
      }
      ++processed;
    }
  }

  /**
   * \brief deletes all data which is related to the tid
   * is called when a thread finishes (either from \ref join() or from \ref
   * finish()) \note Not Threadsafe
   */
  void cleanup(VectorClock<>::Thread_Num th_num) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
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
    VectorClock<>::thread_nums.emplace(th_num);
  }

  inline void clearVarStates() {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (_flag_removeUselessVarStates) {
      static uint32_t should_call = 1;
      static uint32_t consider_useless = 1;
      if (should_call >= consider_useless) {
        VectorClock<>::Clock tmp = removeUselessVarStates(_last_min_th_clock);
        if (_last_min_th_clock == tmp) {
          consider_useless *= 2;
        } else {
          _last_min_th_clock = tmp;
          consider_useless = 1;
        }
        should_call = 1;
        return;
      } else {
        should_call++;
      }
    }
    if (_flag_removeDropSubMaps) {
      constexpr size_t mask = 0xFull;
      static size_t index = 0;
      vars.dropSubMap((index & mask));
      index++;
      if (log_flag) {
        log_count.dropSubMap_calls++;
      }
    }
    if (_flag_removeRandomVarStates) {
      removeRandomVarStates();
    }
    if (_flag_removeLowestClockVarStates) {
      removeVarStates();
    }
  }

  void removeVarStatesOfThread(VectorClock<>::Thread_Num th_num) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    auto it = vars.begin();
    while (it != vars.end()) {
      if (VectorClock<>::make_th_num(it->second.get_write_id()) == th_num ||
          VectorClock<>::make_th_num(it->second.get_read_id()) == th_num) {
        auto tmp = it;
        it++;
        vars.erase(tmp);

        if (log_flag) {
          log_count.no_VarStates_Of_Thread_removed++;
        }
      } else {
        it++;
      }
    }
  }

  VectorClock<>::Clock removeUselessVarStates(
      VectorClock<>::Clock last_min_th_clock) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (log_flag) {
      log_count.removeUselessVarStates_calls++;
    }
    VectorClock<>::Clock min_th_clock = -1;
    {
      std::lock_guard<LockT> exLockT(g_lock);
      for (auto it = threads.begin(); it != threads.end(); ++it) {
        VectorClock<>::Clock tmp = -1;
        if (threads.size() > it->second->get_length()) {
          tmp = 0;
        } else {
          tmp = it->second->get_min_clock();
        }
        if (tmp < min_th_clock) {
          min_th_clock = tmp;
        }
      }
    }

    if (min_th_clock == last_min_th_clock) return min_th_clock;

    auto it = vars.begin();
    while (it != vars.end()) {  // this might be really slow
      if (min_th_clock > it->second.get_w_clock() &&
          (min_th_clock > it->second.get_r_clock() ||
           it->second.r_id == VarState::R_ID_SHARED)) {
        // if the VarState is in read_shared, it is actually indicated to be
        // removed
        auto tmp = it;
        it++;
        vars.erase(tmp);

        if (log_flag) {
          log_count.no_Useless_VarStates_removed++;
        }
      } else {
        it++;
      }
    }

    return min_th_clock;
  }

  void removeRandomVarStates() {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (log_flag) {
      log_count.removeRandomVarStates_calls++;
    }

    auto it = vars.begin();
    // number of VarStates to consider at once for choosing
    std::size_t no_VarStates = 4;
    std::size_t pos = 0;
    std::size_t vsize = vars.size();

    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now)
                  .time_since_epoch()
                  .count();
    std::mt19937 _gen{(unsigned int)ms};
    std::uniform_int_distribution<int> dist(0, no_VarStates - 1);

    while (pos < vsize - 1) {
      std::size_t random = dist(_gen);
      // TODO: needs adjustment, the threshold. might be too low
      pos += random;
      if (pos > vsize - 1) break;

      std::advance(it, random);  // might skip vars.end();
                                 // this is why we have the pos

      // remove the variable, we don't care if it is read_shared or not
      auto random_it = it;
      pos++;
      if (pos <= vsize - 1) {
        it++;
      }
      vars.erase(random_it);

      if (log_flag) {
        log_count.no_Random_VarStates_removed++;
      }
    }
  }

  void removeVarStates() {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (log_flag) {
      log_count.removeVarStates_calls++;
    }

    auto it = vars.begin();
    std::size_t pos = 0;
    VectorClock<>::Clock min_clock = -1;
    auto remove_it = it;

    while (it != vars.end()) {
      // gather data from 3 variables and remove the one with the lowest clock.
      if (it->second.get_w_clock() < min_clock) {
        remove_it = it;
        min_clock = it->second.get_w_clock();
      }
      it++;
      pos++;
      if (pos == 3) {  // 5 would be way too small. e.g. 2000/10000
        pos = 0;
        vars.erase(remove_it);
        remove_it = it;
        min_clock = -1;

        if (log_flag) {
          log_count.no_Useful_VarStates_removed++;
        }
      }
    }
  }

  inline void set_read_write(VarState* var, ThreadState* thr, void* pc) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    VectorClock<>::Thread_Num th_num = thr->get_th_num();

    if (var->_read_write.capacity() < th_num) {
      var->_read_write.reserve(th_num * 2 < 16384 ? th_num * 2 : 16384);
      while (var->_read_write.size() < var->_read_write.capacity()) {
        var->_read_write.emplace_back(std::make_pair(0, 0));
      }
    }

    // th_num start from 1, not from 0, because of something I needed to
    // identify races in a read_shared state
    var->_read_write[th_num - 1] =
        std::make_pair(reinterpret_cast<size_t>(pc),
                       thr->get_stackDepot().get_current_element());
  }

  /// returns a stack trace of a clock for handing it over to drace
  std::list<size_t> return_stack_trace(const VarState& var,
                                       ThreadState* t) const {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    std::lock_guard<ipc::spinlock> lg1(StackTrace::lock);
    auto data = var._read_write[t->get_th_num() - 1];
    return t->get_stackDepot().make_trace(data);
    // A read/write operation was not tracked correctly => return empty
    // stacktrace
    return {};
  }

 public:  // the log counters are public for testing
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
    uint32_t removeUselessVarStates_calls = 0;
    uint32_t removeRandomVarStates_calls = 0;
    uint32_t removeVarStates_calls = 0;
    std::size_t no_Useful_VarStates_removed = 0;
    std::size_t no_Useless_VarStates_removed = 0;
    std::size_t no_Random_VarStates_removed = 0;
    std::size_t no_allocatedVarStates = 0;
    std::size_t dropSubMap_calls = 0;
    std::size_t no_VarStates_Of_Thread_removed = 0;
  } log_count;

 private:
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
    std::cout << "vars_size: " << vars_size << std::endl;
    std::cout << "removeUselessVarStates calls: "
              << log_count.removeUselessVarStates_calls << std::endl;
    std::cout << "removeRandomVarStates calls: "
              << log_count.removeRandomVarStates_calls << std::endl;
    std::cout << "removeVarStates calls: " << log_count.removeVarStates_calls
              << std::endl;
    std::cout << "dropSubMap_calls calls: " << log_count.dropSubMap_calls
              << std::endl;
    std::cout << "no_Useful_VarStates_removed: "
              << log_count.no_Useful_VarStates_removed << std::endl;
    std::cout << "no_Useless_VarStates_removed: "
              << log_count.no_Useless_VarStates_removed << std::endl;
    std::cout << "no_Random_VarStates_removed: "
              << log_count.no_Random_VarStates_removed << std::endl;
    std::cout << "no_VarStates_Of_Thread_removed: "
              << log_count.no_VarStates_Of_Thread_removed << std::endl;
    std::cout << "number of allocated VarStates: "
              << log_count.no_allocatedVarStates << std::endl;
    std::cout << "wr_race: " << log_count.wr_race << std::endl;
    std::cout << "rw_sh_race: " << log_count.rw_sh_race << std::endl;
    std::cout << "ww_race: " << log_count.ww_race << std::endl;
    std::cout << "rw_ex_race: " << log_count.rw_ex_race << std::endl;
    std::cout << "s_spinlock_counter: " << ipc::s_spinlock_counter << std::endl;
    std::cout << std::endl;
  }
};
}  // namespace detector
}  // namespace drace
#endif  // !FASTTRACK_H
