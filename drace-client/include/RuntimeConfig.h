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

#include <string>
#include <vector>

namespace drace {
/// Runtime parameters
class RuntimeConfig {
 public:
  unsigned sampling_rate{1};
  unsigned instr_rate{1};
  bool lossy{false};
  bool lossy_flush{false};
  bool excl_traces{false};
  bool excl_stack{false};
  bool exclude_master{false};
  bool delayed_sym_lookup{false};
  /// search for annotations in modules of target application
  bool annotations{true};
  unsigned suppression_level{1};
  /** Use external controller */
  bool extctrl{false};
  bool break_on_race{false};
  bool stats_show{false};
  unsigned stack_size{31};
  std::string config_file{"drace.ini"};
  std::string out_file;
  std::string xml_file;
  std::string logfile{"stderr"};
  std::string detector{"tsan"};
  std::string filter_file{"race_suppressions.txt"};

  /// detector options/arguments (assuming no collisions with other DRace flags)
  std::vector<std::string> detector_options;

  // Raw arguments
  int argc;
  const char** argv;

 public:
  /// output current runtime configuration
  void print_config() const;
  /// parse CLI arguments
  void parse_args(int argc, const char** argv);
};
}  // namespace drace
