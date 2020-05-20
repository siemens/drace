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

#include <chrono>
#include <string>
#include <vector>

#ifdef WINDOWS
#include <windows.h>
using file_t = HANDLE;
#else
using file_t = int;
#endif

namespace drace {
/** Target to which all logs are written. Can be any file handle */
extern file_t log_target;

/// Utility functions used in drace modules
namespace util {
/** Returns true if a starts with prefix b */
bool common_prefix(const std::string& a, const std::string& b);

std::vector<std::string> split(const std::string& str,
                               const std::string& delimiter);

std::string dir_from_path(const std::string& fullpath);

std::string basename(const std::string& fullpath);

/// removes the filename extension
std::string without_extension(const std::string& path);

std::string to_iso_time(std::chrono::system_clock::time_point tp);

std::string instr_flags_to_str(uint8_t flags);

constexpr bool is_windows() {
#ifdef WIN32
  return true;
#else
  return false;
#endif
}

constexpr bool is_unix() {
#ifdef UNIX
  return true;
#else
  return false;
#endif
}

template <typename T>
T unsafe_ptr_cast(void* ptr) {
#pragma warning(suppress : 4302 4311)
  return (T)(reinterpret_cast<intptr_t>(ptr));
}
}  // namespace util
}  // namespace drace

// Logging
#define LOG_HELPER(tid, level, msg, ...)                                       \
  do {                                                                         \
    if (drace::log_target != 0)                                                \
      dr_fprintf(drace::log_target, "[drace][%.5i][%s] " msg "\n", tid, level, \
                 ##__VA_ARGS__);                                               \
  } while (0)

#define LOG(tid, level, msg, ...)                                          \
  do {                                                                     \
    if (tid == 0) {                                                        \
      LOG_HELPER(dr_get_thread_id(dr_get_current_drcontext()), level, msg, \
                 ##__VA_ARGS__);                                           \
    } else if (tid == -1) {                                                \
      LOG_HELPER(0, level, msg, ##__VA_ARGS__);                            \
    } else {                                                               \
      LOG_HELPER(tid, level, msg, ##__VA_ARGS__);                          \
    }                                                                      \
  } while (0)

#if LOGLEVEL > 3
#define LOG_TRACE(tid, msg, ...) LOG(tid, " trace", msg, ##__VA_ARGS__)
#else
#define LOG_TRACE(tid, msg, ...)
#endif

#if LOGLEVEL > 2
#define LOG_NOTICE(tid, msg, ...) LOG(tid, "notice", msg, ##__VA_ARGS__)
#else
#define LOG_NOTICE(tid, msg, ...)
#endif

#if LOGLEVEL > 1
#define LOG_INFO(tid, msg, ...) LOG(tid, "  info", msg, ##__VA_ARGS__)
#else
#define LOG_INFO(tid, msg, ...)
#endif

#if LOGLEVEL > 0
#define LOG_WARN(tid, msg, ...) LOG(tid, "  warn", msg, ##__VA_ARGS__)
#else
#define LOG_WARN(tid, msg, ...)
#endif

#define LOG_ERROR(tid, msg, ...) LOG(tid, " error", msg, ##__VA_ARGS__)
