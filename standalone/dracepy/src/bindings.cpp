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

#include <boost/algorithm/string.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/class.hpp>
#include <boost/python/list.hpp>
#include <boost/python/module.hpp>
#include <string>

#include <Detector.h>
#include "DetectorPy.h"
#include "ThreadStatePy.h"

BOOST_PYTHON_MODULE(dracepy) {
  // clang-format off
  python::class_<DetectorPy, boost::noncopyable>(
      "Detector",
        "race-detection object that uses the specified backend algorithm",
        python::init<const std::string&>())
      .def("init", &DetectorPy::init,
        "initialize the detector. Has to be called before any analysis function.")
      .def("finalize", &DetectorPy::finalize,
        "closes the detection session. A new session can be created with init.")
      .def("fork", &DetectorPy::fork,
        "log a thread fork event")
      .def("join", &DetectorPy::join,
        "log a thread join event")
      .def("name", &DetectorPy::name,
        "return the name of the detector")
      .def("version", &DetectorPy::version,
        "return the version of the detector");

  python::class_<ThreadStatePy>("ThreadState", python::init<Detector::tls_t, Detector*>())
      .def("acquire", &ThreadStatePy::acquire,
        "log a mutex aquire event")
      .def("release", &ThreadStatePy::release,
        "log a mutex release event")
      .def("happens_before", &ThreadStatePy::happens_before,
        "draw a happens before arc")
      .def("happens_after", &ThreadStatePy::happens_after,
        "draw a happens after arc")
      .def("func_enter", &ThreadStatePy::func_enter,
        "log a function call")
      .def("func_exit", &ThreadStatePy::func_exit,
        "log the return from a function")
      .def("read", &ThreadStatePy::read,
        "log a memory read")
      .def("write", &ThreadStatePy::write,
        "log a memory write");

  python::class_<Detector::AccessEntry>("AccessEntry")
      .add_property("write", &Detector::AccessEntry::write)
      .add_property("thread_id", &Detector::AccessEntry::thread_id)
      .add_property("address", &Detector::AccessEntry::accessed_memory);
  // clang-format on
};
