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
#include <boost/python.hpp>
#include <vector>

namespace python = boost::python;

// declare static python callback
python::object DetectorPy::_pycb;

DetectorPy::DetectorPy(const std::string& detector) {
  // TODO: load detector library and bind
  _det.reset(CreateDetector());
}

void DetectorPy::init(const std::string& args, python::object callback) {
  // TODO: take list of strings instead of single string
  // arguments
  std::vector<std::string> tokens;
  boost::split(tokens, args, [](char c) { return c == ' '; });
  std::vector<const char*> argv;
  for (const auto& s : tokens) {
    argv.push_back(s.c_str());
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
