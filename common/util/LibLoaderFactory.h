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

#include <memory>
#include <string>

#include "LibraryLoader.h"
#ifdef WIN32
#include "WindowsLibLoader.h"
#else
#include "UnixLibLoader.h"
#endif

namespace util {
class LibLoaderFactory {
 public:
  static std::shared_ptr<LibraryLoader> getLoader() {
#ifdef WIN32
    return std::make_shared<WindowsLibLoader>();
#else
    return std::make_shared<UnixLibLoader>();
#endif
  }

  /// return the filename extension of a shared module (e.g. '.dll')
  static std::string getModuleExtension() {
#ifdef WIN32
    return ".dll";
#else
    return ".so";
#endif
  }

  /// return the prefix of a shared module (e.g. 'lib')
  static std::string getModulePrefix() {
#ifdef WIN32
    return "";
#else
    return "lib";
#endif
  }
};
}  // namespace util
