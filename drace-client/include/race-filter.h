
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

#ifndef RACEFILTER_H
#define RACEFILTER_H

#include <detector/Detector.h>
#include <race/DecoratedRace.h>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace drace {

/**
 * \brief Filter data-races based on various criteria
 *
 * The class itself is not threadsafe. When using in combination with the \ref
 * RaceCollector, make sure to synchronize calls to non const data structures
 * using the RaceCollector mutex.
 *
 * \see RaceCollector::get_mutex()
 */
class RaceFilter {
  std::vector<std::string> rtos_list;  // race top of stack
  std::vector<std::string> race_list;
  std::map<uint64_t, unsigned, std::greater<uint64_t>> addr_list;

  void normalize_string(std::string &expr);

  /**
   * \brief check if the race contains symbols which are suppressed
   * \return true if suppressed
   * \note threadsafe
   */
  bool check_race(const drace::race::DecoratedRace &race) const;

  /**
   * \brief check if the top-of-stack symbol is suppressed
   * \return true if suppressed
   * \note threadsafe
   */
  bool check_rtos(const drace::race::DecoratedRace &race) const;

  /**
   * \brief check if the racy-address belongs to an excluded module
   * \return true if suppressed
   * \note threadsafe
   */
  bool in_silent_module(const Detector::Race *race) const;

  /**
   * \brief at least one of the callstacks contains garbage data
   * \return true if suppressed
   * \note threadsafe
   */
  bool stack_implausible(const Detector::Race *race) const;

  /**
   * \brief check if the racy memory-location is suppressed
   * \return true if suppressed
   * \note not-threadsafe
   */
  bool addr_is_suppressed(uint64_t addr) const;

  void init(std::istream &content);

 public:
  RaceFilter() = default;

  /// constructor which takes a supression list as input stream
  explicit RaceFilter(std::istream &content);

  /// constructor which takes a filename and path
  RaceFilter(const std::string &filename, const std::string &hint = {});

  /**
   * \brief return true if race should be suppressed
   *
   * \note not-threadsafe
   */
  bool check_suppress(const drace::race::DecoratedRace &race) const;

  /**
   * \brief check if the information in the race is plausible
   * \return true if suppressed
   * \note threadsafe
   */
  bool check_implausible(const Detector::Race *r) const;

  /// print a list of suppression criteria
  void print_list() const;

  /**
   * \brief add an address to the suppression list
   *
   * \note not-threadsafe
   *
   * \todo use interval trees and exact addresses
   */
  void suppress_addr(uint64_t addr,
                     /// size in bytes
                     unsigned size = 8);
};

}  // namespace drace

#endif  // RACEFILTER_H
