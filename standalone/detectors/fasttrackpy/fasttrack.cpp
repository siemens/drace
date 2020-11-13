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

#include <Detector.h>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace python = boost::python;

class ThreadStatePy {
 public:
  ThreadStatePy(Detector::tls_t tls, Detector* det) : _tls{tls}, _det{det} {}

  void acquire(size_t mutex, int recursive, bool write) {
    _det->acquire(_tls, reinterpret_cast<void*>(mutex), recursive, write);
  }
  void release(void* mutex, bool write) { _det->release(_tls, mutex, write); }
  void happens_before(size_t identifier) {
    _det->happens_before(_tls, reinterpret_cast<void*>(identifier));
  }
  void happens_after(size_t identifier) {
    _det->happens_after(_tls, reinterpret_cast<void*>(identifier));
  }
  void func_enter(size_t pc) {
    _det->func_enter(_tls, reinterpret_cast<void*>(pc));
  }
  void func_exit() { _det->func_exit(_tls); }
  void read(size_t pc, size_t addr, size_t size) {
    _det->read(_tls, reinterpret_cast<void*>(pc), reinterpret_cast<void*>(addr),
               size);
  }
  void write(size_t pc, size_t addr, size_t size) {
    _det->write(_tls, reinterpret_cast<void*>(pc),
                reinterpret_cast<void*>(addr), size);
  }

 private:
  Detector::tls_t _tls;
  Detector* _det;
};

class DetectorPy {
 public:
  using tls_t = python::object;
  explicit DetectorPy(const std::string& detector) {
    // TODO: load detector library and bind
    _det.reset(CreateDetector());
  }

  ~DetectorPy() { finalize(); }

  void init(const std::string& args, python::object callback) {
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

  void finalize() {
    if (_active) {
      _det->finalize();
    }
    _active = false;
  }

  python::object fork(Detector::tid_t parent, Detector::tid_t child) {
    Detector::tls_t tls;
    _det->fork(parent, child, &tls);
    return python::object(ThreadStatePy{tls, _det.get()});
  }

  void join(Detector::tid_t parent, Detector::tid_t child) {
    _det->join(parent, child);
  }

  const char* name() { return _det->name(); };
  const char* version() { return _det->version(); };

 private:
  std::unique_ptr<Detector> _det;
  bool _active{false};
  // detector does not support context.
  // TODO: remove static once i#82 is implemented
  static python::object _pycb;
};

// declare static python callback
python::object DetectorPy::_pycb;

BOOST_PYTHON_MODULE(fasttrackpy) {
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
