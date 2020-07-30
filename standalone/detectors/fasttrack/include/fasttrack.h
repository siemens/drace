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
#include <iomanip>
#include <iostream>
#include <mutex>  // for lock_guard
#include <shared_mutex>
#include "parallel_hashmap/phmap.h"
#include "stacktrace.h"
#include "threadstate.h"
#include "varstate.h"
#include "xvector.h"

#define PROF_INFO false
#include "prof.h"

#define MAKE_OUTPUT false
#define REGARD_ALLOCS true
#define POOL_ALLOC false

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
  /// these maps hold the various state objects together with the identifiers
  phmap::parallel_flat_hash_map<size_t, size_t> allocs;
  // we have to use a node hash map here, as we access the nodes from multiple
  // threads, while the map might grow in the meantime. \todo for memory
  // efficiency and locality, better use (userspace) RW-locks and use
  //       the flat version of the map
  phmap::parallel_node_hash_map<size_t, VarState> vars;
  // number of locks, threads is expected to be < 1000, hence use one map
  // (without submaps)
  phmap::flat_hash_map<void*, VectorClock<>> locks;
  phmap::flat_hash_map<tid_ft, ts_ptr> threads;
  phmap::parallel_flat_hash_map<void*, VectorClock<>> happens_states;

  /// holds the callback address to report a race to the drace-main
  Callback clb;

  /// switch logging of read/write operations
  bool log_flag = true;

  /// internal statistics
  struct log_counters {
    uint32_t read_ex_same_epoch = 0;
    uint32_t read_sh_same_epoch = 0;
    uint32_t read_shared = 0;
    uint32_t read_exclusive = 0;
    uint32_t read_share = 0;
    uint32_t write_same_epoch = 0;
    uint32_t write_exclusive = 0;
    uint32_t write_shared = 0;
  } log_count;

  /// central lock, used for accesses to global tables except vars (order: 1)
  LockT g_lock;  // global Lock

  /// spinlock to protect accesses to vars table (order: 2)
  mutable ipc::spinlock vars_spl;

  /**
   * \brief report a data-race back to DRace
   * \note the function itself must not use locks
   * \note Invariant: this function requires a lock the following global tables:
   *                  threads
   */
  void report_race(uint32_t thr1, uint32_t thr2, bool wr1, bool wr2,
                   const VarState& var, size_t address) const {
    auto it = threads.find(thr1);
    auto it2 = threads.find(thr2);
    auto it_end = threads.end();

    if (it == it_end ||
        it2 == it_end) {  // if thread_id is, because of finishing, not in stack
                          // traces anymore, return
      return;
    }
    std::list<size_t> stack1(
        std::move(it->second->get_stackDepot().return_stack_trace(address)));
    std::list<size_t> stack2(
        std::move(it2->second->get_stackDepot().return_stack_trace(address)));

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
    access1.access_size = var.size;
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
    access2.access_size = var.size;
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
   * \brief Wrapper for report_race to use const qualifier on
   *        wrapped function
   */
  void report_race_locked(uint32_t thr1, uint32_t thr2, bool wr1, bool wr2,
                          const VarState& var, size_t addr) {
    std::lock_guard<LockT> lg(g_lock);
    report_race(thr1, thr2, wr1, wr2, var, addr);
  }

  /**
   * \brief takes care of a read access
   * \note works only on calling-thread and var object, not on any list
   */
  void read(ThreadState* t, VarState* v, size_t addr) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (t->return_own_id() ==
        v->get_read_id()) {  // read same epoch, same thread;
      if (log_flag) {
        log_count.read_ex_same_epoch++;
      }
      return;
    }

    size_t id = t->return_own_id();
    uint32_t tid = t->get_tid();

    if (v->is_read_shared() &&
        v->get_vc_by_thr(tid) == id) {  // read shared same epoch
      if (log_flag) {
        log_count.read_sh_same_epoch++;
      }
      return;
    }

    if (v->is_wr_race(t)) {  // write-read race
      report_race_locked(v->get_w_tid(), tid, true, false, *v, addr);
    }

    // update vc
    if (!v->is_read_shared()) {
      if (v->get_read_id() == VarState::VAR_NOT_INIT ||
          (v->get_r_tid() ==
           tid))  // read exclusive->read of same thread but newer epoch
      {
        if (log_flag) {
          log_count.read_exclusive++;
        }
        v->update(false, id);
      } else {  // read gets shared
        if (log_flag) {
          log_count.read_share++;
        }
        v->set_read_shared(id);
      }
    } else {  // read shared
      if (log_flag) {
        log_count.read_shared++;
      }
      v->update(false, id);
    }
  }

  /**
   * \brief takes care of a write access
   * \note works only on calling-thread and var object, not on any list
   */
  void write(ThreadState* t, VarState* v, size_t addr) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    if (t->return_own_id() == v->get_write_id()) {  // write same epoch
      if (log_flag) {
        log_count.write_same_epoch++;
      }
      return;
    }

    if (v->get_write_id() ==
        VarState::VAR_NOT_INIT) {  // initial write, update var
      if (log_flag) {
        log_count.write_exclusive++;
      }
      v->update(true, t->return_own_id());
      return;
    }

    uint32_t tid = t->get_tid();

    // tids are different and write epoch greater or equal than known epoch of
    // other thread
    if (v->is_ww_race(t))  // write-write race
    {
      report_race_locked(v->get_w_tid(), tid, true, true, *v, addr);
    }

    if (!v->is_read_shared()) {
      if (log_flag) {
        log_count.write_exclusive++;
      }
      if (v->is_rw_ex_race(t))  // read-write race
      {
        report_race_locked(v->get_r_tid(), t->get_tid(), false, true, *v, addr);
      }
    } else {  // come here in read shared case
      if (log_flag) {
        log_count.write_shared++;
      }
      uint32_t act_tid = v->is_rw_sh_race(t);
      if (act_tid != 0)  // read shared read-write race
      {
        report_race_locked(act_tid, tid, false, true, *v, addr);
      }
    }
    v->update(true, t->return_own_id());
  }

  /**
   * \brief creates a new variable object (is called, when var is read or
   * written for the first time) \note Invariant: vars table is locked
   */
  inline auto create_var(size_t addr, size_t size) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    return vars.emplace(addr, static_cast<uint16_t>(size)).first;
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
  ThreadState* create_thread(VectorClock<>::TID tid, ts_ptr parent = nullptr) {
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
    std::cout << "s_spinlock_counter: " << ipc::s_spinlock_counter << std::endl;
              
  }

  /**
   * \brief deletes all data which is related to the tid
   *
   * is called when a thread finishes (either from \ref join() or from \ref
   * finish())
   *
   * \note Not Threadsafe
   */
  void cleanup(uint32_t tid) {
#if PROF_INFO
    PROF_FUNCTION();
#endif
    {
      for (auto it = locks.begin(); it != locks.end(); ++it) {
        it->second.delete_vc(tid);
      }

      for (auto it = threads.begin(); it != threads.end(); ++it) {
        it->second->delete_vc(tid);
      }

      for (auto it = happens_states.begin(); it != happens_states.end(); ++it) {
        it->second.delete_vc(tid);
      }
    }
  }

  void parse_args(int argc, const char** argv) {
    int processed = 1;
    while (processed < argc) {
      if (strcmp(argv[processed], "--stats") == 0) {
        log_flag = true;
        return;
      } else {
        ++processed;
      }
    }
  }

 public:
  explicit Fasttrack() = default;

  bool init(int argc, const char** argv, Callback rc_clb) final {
    parse_args(argc, argv);
    clb = rc_clb;  // init callback
    return true;
  }

  void finalize() final {
    std::lock_guard<LockT> lg1(g_lock);
    {
      std::lock_guard<ipc::spinlock> lg2(vars_spl);
      vars.clear();
    }
    locks.clear();
    happens_states.clear();
    allocs.clear();
    threads.clear();

#if PROF_INFO
    ProfTimer::Print();
#endif

    if (log_flag) {
      process_log_output();
    }
  }

  void read(tls_t tls, void* pc, void* addr, size_t size) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
#if PROF_INFO
    PROF_START_BLOCK("set_read_write")
#endif
    thr->get_stackDepot().set_read_write((size_t)(addr),
                                         reinterpret_cast<size_t>(pc));

#if PROF_INFO
    PROF_END_BLOCK
#endif
    VarState* var;
    {
      std::lock_guard<ipc::spinlock> exLockT(vars_spl);
#if PROF_INFO
      PROF_START_BLOCK("find")
#endif
      auto it = vars.find((size_t)(addr));
      if (it == vars.end()) {  // create new variable if new
#if MAKE_OUTPUT
        std::cout << "variable is read before written" << std::endl;  // warning
#endif
        it = create_var((size_t)(addr), size);
      }
      var = &(it->second);
#if PROF_INFO
      PROF_END_BLOCK
#endif
    }
    std::lock_guard<ipc::spinlock> lg(var->lock);
    read(thr, var, (size_t)addr);
  }

  void write(tls_t tls, void* pc, void* addr, size_t size) final {
    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);
#if PROF_INFO
    PROF_START_BLOCK("set_read_write")
#endif
    thr->get_stackDepot().set_read_write((size_t)(addr),
                                         reinterpret_cast<size_t>(pc));

#if PROF_INFO
    PROF_END_BLOCK
#endif
    VarState* var;
    {
      std::lock_guard<ipc::spinlock> exLockT(vars_spl);
#if PROF_INFO
      PROF_START_BLOCK("find")
#endif
      auto it = vars.find((size_t)addr);
      if (it == vars.end()) {
        it = create_var((size_t)(addr), size);
      }
      var = &(it->second);
#if PROF_INFO
      PROF_END_BLOCK
#endif
    }
    std::lock_guard<ipc::spinlock> lg(var->lock);
    write(thr, var, (size_t)addr);
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
    ts_ptr par_thread = parent_it->second;
    del_thread->inc_vc();
    // pass incremented clock of deleted thread to parent
    par_thread->update(*del_thread);
    threads.erase(del_thread_it);
    cleanup(child);
  }

  // sync thread vc to lock vc
  void acquire(tls_t tls, void* mutex, int recursive, bool write) final {
    if (recursive >
        1) {   // lock haven't been updated by another thread (by definition)
      return;  // therefore no action needed here as only the invoking thread
               // may have updated the lock
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
    it->second.update(thr);
  }

  void happens_before(tls_t tls, void* identifier) final {
    std::lock_guard<LockT> exLockT(g_lock);
    auto it = happens_states.find(identifier);
    if (it == happens_states.end()) {
      it = create_happens(identifier);
    }

    ThreadState* thr = reinterpret_cast<ThreadState*>(tls);

    thr->inc_vc();  // increment clock of thread and update happens state
    it->second.update(thr->get_tid(), thr->return_own_id());
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
        thr->update(it->second);
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

  void deallocate(tls_t tls, void* addr) final {
#if REGARD_ALLOCS
    size_t address = reinterpret_cast<size_t>(addr);
    size_t end_addr;

    std::lock_guard<LockT> exLockT(g_lock);
    end_addr = address + allocs[address];

    // variable is deallocated so varstate objects can be destroyed
    while (address < end_addr) {
      std::lock_guard<ipc::spinlock> lg(vars_spl);
      if (vars.find(address) != vars.end()) {
        auto& var = vars.find(address)->second;
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
    threads.erase(thread_id);
    cleanup(thread_id);
  }

  /**
   * \note allocation of shadow memory is not required in this
   *       detector. If a standalone version of Fasttrack is used
   *       this function can be ignored
   */
  void map_shadow(void* startaddr, size_t size_in_bytes) final { return; }

  const char* name() final { return "FASTTRACK"; }

  const char* version() final { return "0.0.1"; }
};
}  // namespace detector
}  // namespace drace

#endif  // !FASTTRACK_H
