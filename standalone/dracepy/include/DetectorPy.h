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
#include <Detector.h>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <string>

namespace python = boost::python;

class DetectorPy {
 public:
  explicit DetectorPy(const std::string& detector);
  inline ~DetectorPy() { finalize(); }

  void init(const python::list& args, python::object callback);
  void finalize();

  python::object fork(Detector::tid_t parent, Detector::tid_t child);
  inline void join(Detector::tid_t parent, Detector::tid_t child) {
    _det->join(parent, child);
  }

  inline const char* name() { return _det->name(); };
  inline const char* version() { return _det->version(); };

 private:
  std::unique_ptr<Detector> _det;
  bool _active{false};
  // detector does not support context.
  // TODO: remove static once i#82 is implemented
  static python::object _pycb;
};
