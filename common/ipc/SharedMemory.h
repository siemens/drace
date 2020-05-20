#pragma once
/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <conio.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include <stdexcept>

namespace ipc {
/**
 * Provides a shared memory abstraction for two participating units
 * To synchronize accesses, \cnotify() and \cwait() can be used.
 * Internally this is mapped to two windows events,
 * one for sending and one for receiving.
 */
template <
    /// Type of shared memory. Object is constructed in place
    typename T = byte,
    /// If true, no exceptions are used. To check liveness, use \cvalid()
    bool nothrow = false>
class SharedMemory {
  bool _creator;
  HANDLE _event_in;
  HANDLE _event_out;
  HANDLE _hMapFile;
  T* _buffer{nullptr};

 public:
  SharedMemory(const char* name, bool create = false) : _creator(create) {
    if (_creator) {
      _hMapFile =
          CreateFileMapping(INVALID_HANDLE_VALUE,  // use paging file
                            NULL,                  // default security
                            PAGE_READWRITE,        // read/write access
                            0,  // maximum object size (high-order DWORD)
                            sizeof(T),  // maximum object size (low-order DWORD)
                            name);      // name of mapping object
    } else {
      _hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,  // read/write access
                                  FALSE,  // do not inherit the name
                                  name);
    }

    if (nullptr == _hMapFile) {
      if (nothrow) return;
      throw std::runtime_error("error creating/attaching file mapping");
    }

    _buffer = reinterpret_cast<T*>(
        MapViewOfFile(_hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
    if (nullptr == _buffer) {
      if (nothrow) return;
      throw std::runtime_error("error creating file view");
    }

    // Create Event for notification
    std::string evtname("Local\\");
    evtname += name;

    if (_creator) {
      _event_in = CreateEvent(NULL, false, false, (evtname + "-in").c_str());
      _event_out = CreateEvent(NULL, false, false, (evtname + "-out").c_str());
    } else {
      _event_in = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false,
                            (evtname + "-in").c_str());
      _event_out = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, false,
                             (evtname + "-out").c_str());
    }
    if (_event_in == NULL || _event_out == NULL) {
      if (nothrow) return;
      throw std::runtime_error("error creating notification event");
    }

    new (_buffer) T;
  }

  ~SharedMemory() {
    if (nullptr != _event_in) CloseHandle(_event_in);
    if (nullptr != _event_out) CloseHandle(_event_out);

    // destruct object in buffer;
    if (nullptr != _buffer) _buffer->~T();

    if (nullptr != _buffer) UnmapViewOfFile(_buffer);
    if (nullptr != _hMapFile) CloseHandle(_hMapFile);
  }

  T* get() { return _buffer; }

  void notify() const {
    const HANDLE& evt = _creator ? _event_in : _event_out;
    if (!SetEvent(evt)) {
      if (nothrow) return;
      throw std::runtime_error("error in SetEvent");
    }
  }

  template <typename duration = std::chrono::milliseconds>
  bool wait(const duration& d = std::chrono::milliseconds(100)) const {
    using ms_t = std::chrono::milliseconds;

    const HANDLE& evt = _creator ? _event_out : _event_in;
    DWORD result = WaitForSingleObject(
        evt, (DWORD)(std::chrono::duration_cast<ms_t>(d).count()));
    switch (result) {
        // Event fired
      case WAIT_OBJECT_0:
        return true;
      case WAIT_TIMEOUT:
        return false;
        // An error occurred
      default:
        if (!nothrow) throw std::runtime_error("WaitForSingleObject failed");
        return false;
    }
  }
};

}  // namespace ipc
