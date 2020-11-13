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
#include "AccessEntryUtils.h"
#include "ThreadStatePy.h"

#include <util/LibLoaderFactory.h>
#include <util/LibraryLoader.h>
#include <boost/algorithm/string.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <vector>

using namespace dracepy;
namespace python = boost::python;

DetectorPy::DetectorPy(const std::string& detector, const std::string& path)
    : _loader(util::LibLoaderFactory::getLoader()) {
  // if no path is specified, let system search for library
  // otherwise, search in <path>/<prefix><libname><suffix>
  std::string libname = path.empty() ? "" : path + "/";
  libname += util::LibLoaderFactory::getModulePrefix() + detector +
             util::LibLoaderFactory::getModuleExtension();
  if (!_loader->load(libname.c_str()))
    throw std::runtime_error("could not load library " + libname);

  decltype(CreateDetector)* create_detector = (*_loader)["CreateDetector"];
  if (nullptr == create_detector)
    throw std::runtime_error("incompatible detector (function not found)");

  _det = std::unique_ptr<Detector>(create_detector());
}

void DetectorPy::init(const python::list& args, python::object callback) {
  std::vector<std::string> argstrings;
  std::vector<const char*> argv;
  for (long i = 0; i < python::len(args); ++i) {
    argstrings.push_back(python::extract<std::string>(args[i])());
    argv.push_back(argstrings.back().c_str());
  }
  _pycb = callback;

  // initialize detector
  _det->init(static_cast<int>(argv.size()), argv.data(),
             DetectorPy::handle_race, reinterpret_cast<void*>(this));
  _active = true;
}

void DetectorPy::finalize() {
  if (_active) {
    _det->finalize();
  }
  _active = false;
}

ThreadStatePy DetectorPy::fork(Detector::tid_t parent, Detector::tid_t child) {
  Detector::tls_t tls;
  _det->fork(parent, child, &tls);
  return ThreadStatePy{tls, _det.get()};
}

void DetectorPy::handle_race(const Detector::Race* r, void* ctx) {
  ((DetectorPy*)ctx)->_pycb(r->first, r->second);
}
