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

struct TlsWrapper {
  Detector::tls_t tls;
};

class DetectorWrapper {
 private:
  std::unique_ptr<Detector> det;
  bool active{false};
  // detector does not support context.
  // TODO: remove static once i#82 is implemented
  static python::object pycb;

 public:
  using tls_t = python::object;
  explicit DetectorWrapper(const std::string& detector) {
    // TODO: load detector library and bind
    det.reset(CreateDetector());
  }

  ~DetectorWrapper() { finalize(); }

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
    pycb = callback;

    // initialize detector
    det->init(static_cast<int>(argv.size()), argv.data(),
              [](const Detector::Race* r) {
                auto first = python::object{r->first};
                auto secnd = python::object{r->second};
                // TODO: convert stacks to lists
                DetectorWrapper::pycb(first, secnd);
              });
    active = true;
  }

  void finalize() {
    if (active) {
      det->finalize();
    }
    active = false;
  }
  void acquire(tls_t tls, size_t mutex, int recursive, bool write) {
    det->acquire(python::extract<TlsWrapper>(tls)().tls,
                 reinterpret_cast<void*>(mutex), recursive, write);
  }
  void release(tls_t tls, void* mutex, bool write) {
    det->release(python::extract<TlsWrapper>(tls)().tls, mutex, write);
  }
  void happens_before(tls_t tls, size_t identifier) {
    det->happens_before(python::extract<TlsWrapper>(tls)().tls,
                        reinterpret_cast<void*>(identifier));
  }
  void happens_after(tls_t tls, size_t identifier) {
    det->happens_after(python::extract<TlsWrapper>(tls)().tls,
                       reinterpret_cast<void*>(identifier));
  }
  void func_enter(tls_t tls, size_t pc) {
    det->func_enter(python::extract<TlsWrapper>(tls)().tls,
                    reinterpret_cast<void*>(pc));
  }
  void func_exit(tls_t tls) {
    det->func_exit(python::extract<TlsWrapper>(tls)().tls);
  }
  void read(tls_t tls, size_t pc, size_t addr, size_t size) {
    det->read(python::extract<TlsWrapper>(tls)().tls,
              reinterpret_cast<void*>(pc), reinterpret_cast<void*>(addr), size);
  }
  void write(tls_t tls, size_t pc, size_t addr, size_t size) {
    det->write(python::extract<TlsWrapper>(tls)().tls,
               reinterpret_cast<void*>(pc), reinterpret_cast<void*>(addr),
               size);
  }

  python::object fork(Detector::tid_t parent, Detector::tid_t child) {
    Detector::tls_t tls;
    det->fork(parent, child, &tls);
    return python::object(TlsWrapper{std::move(tls)});
  }

  void join(Detector::tid_t parent, Detector::tid_t child) {
    det->join(parent, child);
  }

  const char* name() { return det->name(); };
  const char* version() { return det->version(); };
};

// declare static python callback
python::object DetectorWrapper::pycb;

BOOST_PYTHON_MODULE(fasttrackpy) {
  python::class_<DetectorWrapper, boost::noncopyable>(
      "Detector", python::init<const std::string&>())
      .def("init", &DetectorWrapper::init)
      .def("finalize", &DetectorWrapper::finalize)
      .def("acquire", &DetectorWrapper::acquire)
      .def("release", &DetectorWrapper::release)
      .def("happens_before", &DetectorWrapper::happens_before)
      .def("happens_after", &DetectorWrapper::happens_after)
      .def("func_enter", &DetectorWrapper::func_enter)
      .def("func_exit", &DetectorWrapper::func_exit)
      .def("read", &DetectorWrapper::read)
      .def("write", &DetectorWrapper::write)
      .def("fork", &DetectorWrapper::fork)
      .def("join", &DetectorWrapper::join)
      .def("name", &DetectorWrapper::name)
      .def("version", &DetectorWrapper::version);

  python::class_<TlsWrapper>("Tls", python::init<>());

  python::class_<Detector::AccessEntry>("AccessEntry")
      .add_property("write", &Detector::AccessEntry::write)
      .add_property("thread_id", &Detector::AccessEntry::thread_id)
      .add_property("address", &Detector::AccessEntry::accessed_memory);
};
