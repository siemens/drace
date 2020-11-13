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
#include <detector/Detector.h>

#include "ipc/DrLock.h"
#include "race-filter.h"
#include "race/DecoratedRace.h"
#include "sink/sink.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>

namespace drace {

// forward-decls
class RaceFilter;

/**
 * \brief Singleton to collect all detected races and to manage symbol resolving
 *
 * Note for developers: Be very cautious with concurrency in this class.
 * Locking has to be avoided if possible, as this might interfere with
 * locks held by the application.
 */
class RaceCollector {
 public:
  /// Maximum number of races to collect in delayed mode
  static constexpr int MAX = 200;
  /// Exact type of mutex (implements \ref std::mutex interface)
  using MutexT = DrLock;

 private:
  static RaceCollector* _instance;

 private:
  using RaceCollectionT = std::vector<race::DecoratedRace>;
  using entry_t = race::DecoratedRace;
  using clock_t = std::chrono::high_resolution_clock;
  using tp_t = decltype(clock_t::now());

  RaceCollectionT _races;
  /// guards all accesses to the _races container
  MutexT _races_lock;
  unsigned long _race_count{0};

  bool _delayed_lookup{false};
  std::shared_ptr<symbol::Symbols> _syms;
  tp_t _start_time;
  std::set<uintptr_t> _racy_stacks;

  std::vector<std::shared_ptr<sink::Sink>> _sinks;

  std::shared_ptr<RaceFilter> _filter;

 public:
  RaceCollector(bool delayed_lookup, std::shared_ptr<symbol::Symbols> symbols,
                std::shared_ptr<RaceFilter> filter)
      : _delayed_lookup(delayed_lookup),
        _syms(symbols),
        _start_time(clock_t::now()),
        _filter(filter)

  {
    _races.reserve(1);
    _instance = this;
  }

  ~RaceCollector() {}

  /**
   * \brief register a sink that is notified on each race
   * \note not-threadsafe
   */
  inline void register_sink(const std::shared_ptr<sink::Sink>& sink) {
    _sinks.push_back(sink);
  }

  /**
   * \brief Adds a race and updates histogram
   * \note threadsafe
   */
  void add_race(const Detector::Race* r);

  /**
   * \brief Resolves all unresolved race entries
   * \note threadsafe
   */
  void resolve_all();

  /**
   * \brief In delayed mode, return data-races. Otherwise return empty container
   *
   * \note not-threadsafe
   */
  inline const std::vector<race::DecoratedRace>& get_races() const {
    return _races;
  }

  /**
   * \brief return the number of observed races
   */
  inline unsigned long num_races() const { return _race_count; }

  /**
   * \brief get a reference to the filtering object
   */
  inline RaceFilter& get_racefilter() { return *_filter; }

  /**
   * \brief This function provides a callback to the \ref
   * RaceCollector::add_race on the singleton object. As we have to pass this
   * callback to as a function pointer to c, we cannot use \ref std::bind
   */
  static void race_collector_add_race(const Detector::Race* r, void* context);

  /**
   * \brief get instance to this singleton
   */
  inline static RaceCollector& get_instance() { return *_instance; }

  /**
   * \brief get race-collector mutex
   */
  inline MutexT& get_mutex() { return _races_lock; }

 private:
  /**
   * suppress this race if similar race is already reported
   * \return true if race is suppressed
   * \note   not-threadsafe
   */
  bool filter_duplicates(const Detector::Race* r);

  /**
   * Takes a detector Access Entry, resolves symbols and converts it to a
   * ResolvedAccess */
  void resolve_symbols(race::ResolvedAccess& ra);

  /** resolve a single race */
  void resolve_race(race::DecoratedRace& race);

  /**
   * forward a single race to the sinks
   *
   * \note not-threadsafe
   */
  inline void forward_race(const race::DecoratedRace& race) const {
    for (const auto& s : _sinks) {
      s->process_single_race(race);
    }
  }
};

}  // namespace drace
