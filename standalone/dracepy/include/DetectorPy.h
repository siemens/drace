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
#include <util/LibLoaderFactory.h>
#include <util/LibraryLoader.h>

#include <boost/python/dict.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <memory>
#include <string>

namespace python = boost::python;

class DetectorPy {
 public:
  explicit DetectorPy(const std::string& detector,
                      const std::string& path = {});
  inline ~DetectorPy() { finalize(); }

  void init(const python::list& args, python::object callback);
  void finalize();

  python::object fork(Detector::tid_t parent, Detector::tid_t child);
  inline void join(Detector::tid_t parent, Detector::tid_t child) {
    _det->join(parent, child);
  }

  inline const char* name() { return _det->name(); };
  inline const char* version() { return _det->version(); };

  static void handle_race(const Detector::Race* r);

 private:
  std::shared_ptr<util::LibraryLoader> _loader;
  std::unique_ptr<Detector> _det;
  bool _active{false};
  // detector does not support context.
  // TODO: remove static once i#82 is implemented
  static python::object _pycb;
};
