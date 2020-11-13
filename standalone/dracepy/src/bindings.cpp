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
#include <boost/python.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/list.hpp>
#include <string>

#include <Detector.h>
#include "DetectorPy.h"
#include "ThreadStatePy.h"

BOOST_PYTHON_MODULE(dracepy) {
  // clang-format off
  python::class_<DetectorPy, boost::noncopyable>(
      "Detector", python::init<const std::string&>())
      .def("init", &DetectorPy::init)
      .def("finalize", &DetectorPy::finalize)
      .def("fork", &DetectorPy::fork)
      .def("join", &DetectorPy::join)
      .def("name", &DetectorPy::name)
      .def("version", &DetectorPy::version);

  python::class_<ThreadStatePy>("ThreadState", python::init<Detector::tls_t, Detector*>())
      .def("acquire", &ThreadStatePy::acquire)
      .def("release", &ThreadStatePy::release)
      .def("happens_before", &ThreadStatePy::happens_before)
      .def("happens_after", &ThreadStatePy::happens_after)
      .def("func_enter", &ThreadStatePy::func_enter)
      .def("func_exit", &ThreadStatePy::func_exit)
      .def("read", &ThreadStatePy::read)
      .def("write", &ThreadStatePy::write);

  python::class_<Detector::AccessEntry>("AccessEntry")
      .add_property("write", &Detector::AccessEntry::write)
      .add_property("thread_id", &Detector::AccessEntry::thread_id)
      .add_property("address", &Detector::AccessEntry::accessed_memory);
  // clang-format on
};
