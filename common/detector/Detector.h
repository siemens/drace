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

#include <cstddef>
#include <cstdint>
#include <utility>

// this is a header only target, hence we cannot rely on cmake
// to generate the export headers.
#ifdef WIN32
#ifdef DETECTOR_EXPORTS
#define DETECTOR_EXPORT __declspec(dllexport)
#else
#define DETECTOR_EXPORT __declspec(dllimport)
#endif
#else
#define DETECTOR_EXPORT
#endif

/**
    \brief Interface for a DRace compatible race detector

    Premises for a implementation of a detector back-end:

    - Every thread starts with a initial call of fork()
    - There are no double forks of the same thread as child
    - A read or write will never contain a tid which was not forked
    - A read can happen before a write
    - A read and a write with the same TID will never arrive concurrently
    - A happens after may arrive before a corresponding happens before arrives
    - A lock may be be released, before it will be acquired
    - the last three bullet points must not cause a crash
*/
class Detector {
 public:
  typedef unsigned long tid_t;

  /// type of a pointer to an entry in the thread local storage
  typedef void* tls_t;

  static constexpr int max_stack_size = 16;

  /// A single memory access
  struct AccessEntry {
    bool write;
    bool onheap{false};
    unsigned thread_id;
    int access_type;
    size_t stack_size{0};
    /// access size in log2 bytes -> access_size = 0 -> log2(0) = 1 byte
    size_t access_size;
    size_t heap_block_size;
    uintptr_t accessed_memory;
    uintptr_t heap_block_begin;
    uintptr_t stack_trace[max_stack_size];  // maybe make it a std::array
  };

  /// A Data-Race is a tuple of two Accesses
  using Race = std::pair<AccessEntry, AccessEntry>;

  /**
   * \brief signature of the callback function
   *
   * The callback is executed on each detected race.
   * If the same race happens multiple times, the detector might (but does not
   * have to) suppress subsequent ones.
   *
   * \note When using the DRace runtime, further filtering is done in the \ref
   * drace::RaceCollector.
   */
  using Callback = void (*)(const Race*);

  /**
   * \brief initialize detector
   *
   * Takes command line arguments and a callback to process a data-race.
   * Type of callback is (const detector::Race*) -> void
   */
  virtual bool init(int argc, const char** argv, Callback rc_clb) = 0;

  /**
   * \brief destruct detector
   *
   * The destructor might be implemented by sub-class to free resources
   *
   * \note before destruction, \ref finalize has to be called
   */
  virtual ~Detector() {}

  /**
   * \brief Maps a new block of shadow memory.
   *
   * All memory accesses have to be inside a mapped block.
   * \note the mapped shadow region has to be in a memory range that is
   * shadowable \todo rework memory mapping, i#11
   */
  virtual void map_shadow(void* startaddr, size_t size_in_bytes) = 0;

  /**
   * \brief Finalizes the detector.
   *
   * After a finalize, a later init must be possible
   */
  virtual void finalize() = 0;

  /// Acquire a mutex
  virtual void acquire(
      /// ptr to thread-local storage of calling thread
      tls_t tls,
      /// ptr to mutex location
      void* mutex,
      /// number of recursive locks (1 for non-recursive mutex)
      int recursive,
      /// true, for RW-mutexes in read-mode false
      bool write) = 0;

  /// Release a mutex
  virtual void release(
      /// ptr to thread-local storage of calling thread
      tls_t tls,
      /// ptr to mutex location
      void* mutex,
      /// true, for RW-mutexes in read-mode false
      bool write) = 0;

  /// Enter a function (push stack entry)
  virtual void func_enter(tls_t tls, void* pc) = 0;

  /// Leave a function (pop stack entry)
  virtual void func_exit(tls_t tls) = 0;

  /// Draw a happens-before edge between thread and identifier (can be stubbed)
  virtual void happens_before(tls_t tls, void* identifier) = 0;
  /// Draw a happens-after edge between thread and identifier (can be stubbed)
  virtual void happens_after(tls_t tls, void* identifier) = 0;

  /// Log a read access
  virtual void read(
      /// ptr to thread-local storage of calling thread
      tls_t tls,
      /// current program counter
      void* pc,
      /// memory location
      void* addr,
      /// access size log2 (bytes)
      size_t size) = 0;

  /// Log a write access
  virtual void write(
      /// ptr to thread-local storage of calling thread
      tls_t tls,
      /// current program counter
      void* pc,
      /// memory location
      void* addr,
      /// access size log2 (bytes)
      size_t size) = 0;

  /// Log a memory allocation
  virtual void allocate(
      /// ptr to thread-local storage of calling thread
      tls_t tls,
      /// current instruction pointer
      void* pc,
      /// begin of allocated memory block
      void* addr,
      /// size of memory block in bytes
      size_t size) = 0;

  /// Log a memory deallocation
  virtual void deallocate(
      /// ptr to thread-local storage of calling thread
      tls_t tls,
      /// begin of memory block
      void* addr) = 0;

  /// Log a thread-creation event
  virtual void fork(
      /// id of parent thread
      tid_t parent,
      /// id of child thread
      tid_t child,
      /// out parameter for tls data
      tls_t* tls) = 0;

  /// Log a thread join event
  virtual void join(
      /// id of parent thread
      tid_t parent,
      /// id of child thread
      tid_t child) = 0;

  /// Log a thread detach event
  virtual void detach(tls_t tls, tid_t thread_id) = 0;

  /// Log a thread exit event (detached thread)
  virtual void finish(tls_t tls, tid_t thread_id) = 0;

  /// Return name of detector
  virtual const char* name() = 0;

  /// Return version of detector
  virtual const char* version() = 0;
};

/// create a new detector instance
extern "C" DETECTOR_EXPORT Detector* CreateDetector();

extern "C" DETECTOR_EXPORT void Read_Nvrt(void* detector, void* tls, void* pc,
                                          void* addr, size_t size);

extern "C" DETECTOR_EXPORT void Write_Nvrt(void* detector, void* tls, void* pc,
                                           void* addr, size_t size);

extern "C" DETECTOR_EXPORT void FuncEnter_Nvrt(void* detector, void* tls,
                                                   void* pc);

extern "C" DETECTOR_EXPORT void FuncExit_Nvrt(void* detector, void* tls);
