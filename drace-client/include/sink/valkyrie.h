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

#include "sink.h"
#include "symbol/SymbolLocation.h"
#include "util.h"

#include <dr_api.h>
#include <drutil.h>
#include <tinyxml2.h>

namespace drace {
namespace sink {
/// A race exporter which creates a valgrind valkyrie compatible xml output
class Valkyrie : public Sink {
 public:
  using TimePoint = std::chrono::system_clock::time_point;

 private:
  std::shared_ptr<DrFile> _target;
  int _argc;
  const char** _argv;
  const char* _app;
  TimePoint _start_time;
  TimePoint _end_time;
  unsigned _num_processed{0};
  /// xml writer
  tinyxml2::XMLPrinter _printer;

 private:
  void print_header() {
    auto& p = _printer;
    p.OpenElement("protocolversion");
    p.PushText(4);
    p.CloseElement();

    p.OpenElement("protocoltool");
    p.PushText("helgrind");
    p.CloseElement();

    p.OpenElement("preamble");
    p.OpenElement("line");
    p.PushText("Drace, a thread error detector");
    p.CloseElement();
    p.CloseElement();

    p.OpenElement("pid");
    p.PushText((int)dr_get_process_id());
    p.CloseElement();
#ifndef WINDOWS
    p.OpenElement("ppid");
    p.PushText((int)dr_get_parent_id());
    p.CloseElement();
#endif

    p.OpenElement("tool");
    p.PushText("drace");
    p.CloseElement();
  }

  void print_params() {
    auto& p = _printer;
    p.OpenElement("args");
    // Detector
    p.OpenElement("vargv");
    p.OpenElement("exe");
    p.PushText(_argv[0]);
    p.CloseElement();
    for (int i = 1; i < _argc; ++i) {
      p.OpenElement("arg");
      p.PushText(_argv[i]);
      p.CloseElement();
    }
    p.CloseElement();

    // Application
    p.OpenElement("argv");
    p.OpenElement("exe");
    p.PushText(_app);
    p.CloseElement();
    p.CloseElement();
    p.CloseElement();
  }

  void print_stack(const std::vector<symbol::SymbolLocation>& stack) {
    auto& p = _printer;
    // buffer for pc formatting
    char strbuf[32];

    p.OpenElement("stack");
    int ssize = static_cast<int>(stack.size());
    for (int i = 0; i < ssize; ++i) {
      const auto& f = stack[ssize - 1 - i];
      p.OpenElement("frame");
      // format program counter
      dr_snprintf(strbuf, sizeof(strbuf), "%#018llx", (uintptr_t)f.pc);
      p.OpenElement("ip");
      p.PushText(strbuf);
      p.CloseElement();
      p.OpenElement("obj");
      p.PushText(f.mod_name.c_str());
      p.CloseElement();
      if (!f.sym_name.empty()) {
        p.OpenElement("fn");
        p.PushText(f.sym_name.c_str());
        p.CloseElement();
      }
      if (!f.file.empty()) {
        p.OpenElement("dir");
        p.PushText(util::dir_from_path(f.file).c_str());
        p.CloseElement();
        p.OpenElement("file");
        p.PushText(util::basename(f.file).c_str());
        p.CloseElement();
      }
      if (f.line) {
        p.OpenElement("line");
        p.PushText(std::to_string(f.line).c_str());
        p.CloseElement();
        p.OpenElement("offset");
        p.PushText(std::to_string(f.line_offs).c_str());
        p.CloseElement();
      }
      p.CloseElement();
    }
    p.CloseElement();
  }

  void print_race(const race::DecoratedRace& race) {
    const race::ResolvedAccess& r = race.first;
    const race::ResolvedAccess& r2 = race.second;
    auto& p = _printer;

    // buffer for pc formatting
    constexpr int strbufsize = 256;
    void* drcontext = dr_get_current_drcontext();
    char* strbuf = (char*)dr_thread_alloc(drcontext, strbufsize);

    p.OpenElement("error");
    dr_snprintf(strbuf, strbufsize, "%#04lx", _num_processed++);
    p.OpenElement("unique");
    p.PushText(strbuf);
    p.CloseElement();
    p.OpenElement("tid");
    p.PushText(r2.thread_id);
    p.CloseElement();
    p.OpenElement("threadname");
    p.PushText("Thread");
    p.CloseElement();
    p.OpenElement("kind");
    p.PushText("Race");
    p.CloseElement();
    p.OpenElement("timestamp");
    p.PushAttribute("unit", "ms");
    p.PushText(std::to_string(race.elapsed.count()).c_str());
    p.CloseElement();

    if (!race.resolved_addr.mod_name.empty()) {
      p.OpenElement("resolvedaddress");

      p.OpenElement("modname");
      p.PushText(race.resolved_addr.mod_name.c_str());
      p.CloseElement();

      if (!race.resolved_addr.sym_name.empty()) {
        p.OpenElement("symname");
        p.PushText(race.resolved_addr.sym_name.c_str());
        p.CloseElement();
      }

      if (!race.resolved_addr.file.empty()) {
        p.OpenElement("file");  // TODO: resolve mapping filename to
                                // race.resolved_addr.file
        p.PushText(race.resolved_addr.file.c_str());
        p.CloseElement();
        p.OpenElement("line");  // TODO: line information might not be
                                // available for non-C#
        p.PushText(std::to_string(race.resolved_addr.line).c_str());
        p.CloseElement();
        p.OpenElement("offset");
        p.PushText(std::to_string(race.resolved_addr.line_offs).c_str());
        p.CloseElement();
      }

      p.CloseElement();
    }

    {
      p.OpenElement("xwhat");
      p.OpenElement("text");
      dr_snprintf(
          strbuf, strbufsize,
          "Possible data race during %s of size %d at %#018llx by thread #%d",
          (r2.write ? "write" : "read"), r2.access_size, r2.accessed_memory,
          r2.thread_id);
      p.PushText(strbuf);
      p.CloseElement();
      p.OpenElement("hthreadid");
      p.PushText(r2.thread_id);
      p.CloseElement();
      p.CloseElement();
      print_stack(r2.resolved_stack);
    }
    {
      p.OpenElement("xwhat");
      p.OpenElement("text");
      dr_snprintf(strbuf, strbufsize,
                  "This conflicts with a previous %s of size %d at %#018llx by "
                  "thread #%d",
                  (r.write ? "write" : "read"), r.access_size,
                  r.accessed_memory, r.thread_id);
      p.PushText(strbuf);
      p.CloseElement();
      p.OpenElement("hthreadid");
      p.PushText(r.thread_id);
      p.CloseElement();
      p.CloseElement();
      print_stack(r.resolved_stack);
    }

    p.CloseElement();

    dr_thread_free(drcontext, strbuf, strbufsize);

    flush();
  }

  void print_races(const std::vector<race::DecoratedRace>& races) {
    for (const auto& r : races) {
      print_race(r);
    }
  }

  /**
   * \brief write current xml buffer to file
   */
  void flush() {
    // the buffer is terminated with a '\0', but without the last
    // line-break. Hence, crop the last byte and add a newline
    dr_write_file(_target->get(), _printer.CStr(), _printer.CStrSize() - 1);
    dr_write_file(_target->get(), "\n", 1);
    _printer.ClearBuffer();
  }

 public:
  Valkyrie() = delete;
  Valkyrie(const Valkyrie&) = delete;
  Valkyrie(Valkyrie&&) = default;

  Valkyrie(std::shared_ptr<DrFile> target, int argc, const char** argv,
           const char* app, TimePoint start, TimePoint stop)
      : _target(target),
        _argc(argc),
        _argv(argv),
        _app(app),
        _start_time(start),
        _end_time(stop),
        _printer(0, true) {
    auto& p = _printer;
    p.PushHeader(false, true);
    p.OpenElement("valgrindoutput");
    print_header();
    print_params();

    p.OpenElement("status");
    p.OpenElement("state");
    p.PushText("RUNNING");
    p.CloseElement();

    p.OpenElement("time");
    std::string timestr = util::to_iso_time(_start_time);
    p.PushText(timestr.c_str());
    p.CloseElement();
    p.CloseElement();

    flush();
  }

  Valkyrie& operator=(const Valkyrie&) = delete;
  Valkyrie& operator=(Valkyrie&&) = default;

  virtual ~Valkyrie() {
    auto& p = _printer;
    // in streaming mode, we do not get a end-point, hence assume now
    if (_end_time == TimePoint()) {
      _end_time = std::chrono::system_clock::now();
    }
    p.OpenElement("status");
    p.OpenElement("state");
    p.PushText("FINISHED");
    p.CloseElement();
    p.OpenElement("time");
    std::string timestr = util::to_iso_time(_end_time);
    p.PushText(timestr.c_str());
    p.CloseElement();

    p.OpenElement("duration");
    p.PushAttribute("unit", "ms");
    p.PushText(std::chrono::duration_cast<std::chrono::milliseconds>(
                   _end_time - _start_time)
                   .count());
    p.CloseElement();

    p.CloseElement();  // status
    p.CloseElement();  // valgrindoutput

    flush();
  }

  /**
   * Process / stream a single data race
   */
  virtual void process_single_race(const race::DecoratedRace& race) override {
    print_race(race);
  }

  /**
   * open new document, process all data-races provided in the vector
   * and close it.
   */
  virtual void process_all(
      const std::vector<race::DecoratedRace>& races) override {
    // TODO: Announce Threads
    print_races(races);
  }
};

}  // namespace sink
}  // namespace drace
