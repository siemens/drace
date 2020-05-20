
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

#include "race-filter.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>

#ifdef TESTING
#include <exception>
#else
#include <globals.h>
#include <module/Tracker.h>
#include <util.h>
#endif

namespace drace {

RaceFilter::RaceFilter(std::istream& content) { init(content); }

RaceFilter::RaceFilter(const std::string& filename, const std::string& hint) {
  std::ifstream f;
  f.open(filename);
  if (f.good()) {
    // reads a file which has thread sanitizer suppression file fashion
    init(f);
  } else {
#ifdef TESTING
    throw std::runtime_error("file not found");
#else
    // try to load file next to drace binary
    const std::string file_next_to_drace =
        util::dir_from_path(hint) + "\\" + filename;
    f.open(file_next_to_drace);
    if (f.good()) {
      init(f);
    } else {
      // In DRace, we must not use exceptions,
      // but for testing, it's useful
      LOG_WARN(-1, "File %s not found, no races will be suppressed",
               filename.c_str());
    }
#endif
  }
  f.close();
}

void RaceFilter::init(std::istream& content) {
  std::string entry;
  while (std::getline(content, entry)) {
    // omit lines which start with a '#' -> comments
    if (entry[0] != '#') {
      // valid entries are like 'subject:function_name' -> race_tos:std::foo
      size_t pos = entry.find(':', 0);
      if (pos != std::string::npos) {
        // race_tos top of stack
        if (entry.substr(0, pos) == "race_tos") {
          std::string tmp = entry.substr(pos + 1);
          normalize_string(tmp);
          rtos_list.push_back(tmp);
        }

        // race
        if (entry.substr(0, pos) == "race") {
          std::string tmp = entry.substr(pos + 1);
          normalize_string(tmp);
          race_list.push_back(tmp);
        }
      }
    }
  }
}

void RaceFilter::normalize_string(std::string& expr) {
  size_t pos = 0;

  // make tsan wildcard * to regex wildcard .*
  pos = expr.find('*', 0);
  while (pos != std::string::npos) {
    expr.insert(pos, ".");
    pos = expr.find('*', (pos + 2));
  }

  // wild card at beginning, when no '^' and no existing wild card
  if (expr[0] != '^') {
    if ((expr[0] != '.' && expr[1] != '*')) {
      expr.insert(0, ".*");
    }
  } else {
    expr.erase(0, 1);
  }

  // wild card at end, when no '$' and no existing wild card
  if (expr.back() != '$') {
    if (expr.back() != '*') {
      expr.append(".*");
    }
  } else {
    expr.pop_back();
  }
}

bool RaceFilter::check_suppress(const drace::race::DecoratedRace& race) const {
  if (check_race(race) || check_rtos(race)) {
    return true;
  }
  return false;  // not in list -> do not suppress
}

bool RaceFilter::check_rtos(const drace::race::DecoratedRace& race) const {
  if (!race.first.stack_size || !race.second.stack_size) {
    // no callstacks
    return false;
  }
  // get the top stack elements
  const std::string& top1_sym = race.first.resolved_stack.back().sym_name;
  const std::string& top2_sym = race.second.resolved_stack.back().sym_name;
  for (auto it = rtos_list.begin(); it != rtos_list.end(); ++it) {
    std::regex expr(*it);
    std::smatch m;
    if (std::regex_match(top1_sym.begin(), top1_sym.end(), m, expr) ||
        std::regex_match(top2_sym.begin(), top2_sym.end(), m, expr)) {
      return true;  // at least one regex matched -> suppress
    }
  }
  return false;
}

bool RaceFilter::in_silent_module(const Detector::Race* race) const {
#ifndef TESTING
  // filter out accesses into modules we do not instrument
  const auto module = module_tracker->get_module_containing(
      (app_pc)(race->first.accessed_memory));
  if (module && !(module->instrument & module::INSTR_FLAGS::MEMORY)) {
    return true;
  }
#endif
  return false;
}

bool RaceFilter::stack_implausible(const Detector::Race* race) const {
  if (race->first.stack_size == 0 || race->second.stack_size == 0) return false;

  // PC is null
  if (race->first.stack_trace[race->first.stack_size - 1] == 0x0) return true;
  if (race->second.stack_trace[race->second.stack_size - 1] == 0x0) return true;

  return false;
}

bool RaceFilter::addr_is_suppressed(uint64_t addr) const {
  auto it = addr_list.lower_bound(addr);
  if (it == addr_list.end()) {
    return false;
  }
  if (addr < (it->first + it->second)) {
    return true;
  }
  return false;
}

bool RaceFilter::check_race(const drace::race::DecoratedRace& race) const {
  // get the top stack elements
  auto stack1 = race.first.resolved_stack;
  auto stack2 = race.second.resolved_stack;

  if (addr_is_suppressed(race.first.accessed_memory) ||
      addr_is_suppressed(race.second.accessed_memory)) {
    // memory addr is excluded
    return true;
  }
  for (auto it = race_list.begin(); it != race_list.end(); ++it) {
    std::regex expr(*it);
    std::smatch m;

    for (auto st = stack1.begin(); st != stack1.end(); st++) {
      const std::string& sym = st->sym_name;
      if (std::regex_match(sym.begin(), sym.end(), m, expr)) {
        return true;  // regex matched -> suppress
      }
    }
    for (auto st = stack2.begin(); st != stack2.end(); st++) {
      const std::string& sym = st->sym_name;
      if (std::regex_match(sym.begin(), sym.end(), m, expr)) {
        return true;  // regex matched -> suppress
      }
    }
  }
  return false;
}

void RaceFilter::suppress_addr(uint64_t addr, unsigned size) {
  // round down, round up
  addr_list.emplace(addr, size);
}

void RaceFilter::print_list() const {
  std::cout << "Suppressed TOS functions: " << std::endl;
  for (auto it = rtos_list.begin(); it != rtos_list.end(); ++it) {
    std::cout << *it << std::endl;
  }
  std::cout << "Suppressed functions: " << std::endl;
  for (auto it = race_list.begin(); it != race_list.end(); ++it) {
    std::cout << *it << std::endl;
  }
  std::cout << "Suppressed addresses: " << std::endl;
  for (const auto& addr : addr_list) {
    std::cout << (addr.first << 3) << std::endl;
  }
}

bool RaceFilter::check_implausible(const Detector::Race* r) const {
  if (stack_implausible(r) || in_silent_module(r)) {
    return true;
  }
  return false;
}
}  // namespace drace
