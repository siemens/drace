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

#include "ThreadStatePy.h"

/**
 * \brief Python bindings for DRace detectors
 *
 * This namespace contains the python bindings for all
 * race detectors that implement the \ref Detector interface.
 */
namespace dracepy {

namespace python = boost::python;

/**
 * \brief Python wrapper around \ref Detector
 *
 * The interface is similar to \ref Detector, however all
 * methods that operate on a thread state (first parameter is TLS)
 * are implemented in \ref ThreadStatePy.
 */
class DetectorPy {
 public:
  /**
   * \brief Load a DRace \ref Detector backend
   *
   * All detectors that implement the DRace \ref Detector interface can be
   * loaded.
   *
   * \note as the detector is runtime loaded, it has to be in the
   * library search path of the OS loader
   */
  explicit DetectorPy(
      /// detector filename, without: path, prefix (lib), extension (.dll)
      const std::string& detector,
      /// path to the library (if not specified, use OS loader defaults)
      const std::string& path = {});
  inline ~DetectorPy() { finalize(); }

  /**
   * \brief Wrapper around \ref Detector::init
   *
   * The callback is a python function of type f -> (AccessEntry, AccessEntry)
   */
  void init(const python::list& args, python::object callback);

  /// Wrapper around \ref Detector::finalize
  void finalize();

  /**
   * \brief Wrapper around \ref Detector::fork
   *
   * The returned object contains the ThreadState.
   * All per-thread functions are then invoked on the returned
   * \ref ThreadStatePy instance.
   */
  ThreadStatePy fork(Detector::tid_t parent, Detector::tid_t child);

  /// Wrapper around \ref Detector::join
  inline void join(Detector::tid_t parent, Detector::tid_t child) {
    _det->join(parent, child);
  }

  /// Wrapper around \ref Detector::name
  inline const char* name() { return _det->name(); };

  /// Wrapper around \ref Detector::version
  inline const char* version() { return _det->version(); };

 private:
  static void handle_race(const Detector::Race* r, void* context);

 private:
  std::shared_ptr<util::LibraryLoader> _loader;
  std::unique_ptr<Detector> _det;
  bool _active{false};
  python::object _pycb;
};

}  // namespace dracepy
