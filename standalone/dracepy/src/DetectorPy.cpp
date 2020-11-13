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

#include "DetectorPy.h"
#include "ThreadStatePy.h"

#include <boost/algorithm/string.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <vector>

namespace python = boost::python;

// declare static python callback
python::object DetectorPy::_pycb;

DetectorPy::DetectorPy(const std::string& detector) {
  // TODO: load detector library and bind
  _det.reset(CreateDetector());
}

void DetectorPy::init(const python::list& args, python::object callback) {
  std::vector<std::string> argstrings;
  std::vector<const char*> argv;
  for (long i = 0; i < python::len(args); ++i) {
    argstrings.push_back(python::extract<std::string>(args[i])());
    argv.push_back(argstrings.back().c_str());
  }
  // callback
  _pycb = callback;

  // initialize detector
  _det->init(static_cast<int>(argv.size()), argv.data(),
             [](const Detector::Race* r) {
               auto first = python::object{r->first};
               auto secnd = python::object{r->second};
               // TODO: convert stacks to lists
               DetectorPy::_pycb(first, secnd);
             });
  _active = true;
}

void DetectorPy::finalize() {
  if (_active) {
    _det->finalize();
  }
  _active = false;
}

python::object DetectorPy::fork(Detector::tid_t parent, Detector::tid_t child) {
  Detector::tls_t tls;
  _det->fork(parent, child, &tls);
  return python::object(ThreadStatePy{tls, _det.get()});
}
