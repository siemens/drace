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
#pragma once

#include <detector/Detector.h>
#include <boost/python/list.hpp>

class AccessEntryUtils {
 public:
  static python::list getStackAsList(Detector::AccessEntry* entry) {
    python::list stack;
    for (size_t i = 0; i < entry->stack_size; ++i) {
      stack.append(entry->stack_trace[i]);
    }
    return stack;
  }
};
