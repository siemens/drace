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

#include "util.h"
#include "DrFile.h"
#include "globals.h"
#include "module/Metadata.h"

#include <chrono>
#include <regex>
#include <string>
#include <vector>

#include <date/date.h>

namespace drace {

file_t log_target{0};

namespace util {
bool common_prefix(const std::string& a, const std::string& b) {
  if (!a.size() || !b.size()) {
    return false;
  } else if (a.size() > b.size()) {
    return a.substr(0, b.size()) == b;
  } else {
    return b.substr(0, a.size()) == a;
  }
}

std::vector<std::string> split(const std::string& str,
                               const std::string& delimiter) {
  std::string regstr("([");
  regstr += (delimiter + "]+)");
  std::regex regex{regstr};  // split on space and comma
  std::sregex_token_iterator it{str.begin(), str.end(), regex, -1};
  std::vector<std::string> words{it, {}};
  return words;
}

std::string dir_from_path(const std::string& fullpath) {
  return fullpath.substr(0, fullpath.find_last_of("/\\"));
}

std::string basename(const std::string& fullpath) {
  return fullpath.substr(fullpath.find_last_of("/\\") + 1);
}

std::string without_extension(const std::string& path) {
  size_t pos = path.find_last_of('.');
  return path.substr(0, pos);
}

std::string to_iso_time(std::chrono::system_clock::time_point tp) {
  return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
}

std::string instr_flags_to_str(uint8_t flags) {
  auto _flags = (module::INSTR_FLAGS)flags;
  std::stringstream buffer;
  if (_flags & module::Metadata::INSTR_FLAGS::MEMORY) {
    buffer << "MEMORY ";
  }
  if (_flags & module::Metadata::INSTR_FLAGS::STACK) {
    buffer << "STACK ";
  }
  if (_flags & module::Metadata::INSTR_FLAGS::SYMBOLS) {
    buffer << "SYMBOLS ";
  }
  if (!_flags) {
    buffer << "NONE ";
  }
  return buffer.str();
}
}  // namespace util

}  // namespace drace
